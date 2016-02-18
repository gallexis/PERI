Séance 1:
==

###Manipulation de LED et interrupteur

```#define GPIO_SET( ptr, gpio )``` : Permet d'allumer la LED du peripherique  
```#define GPIO_CLR( ptr, gpio )``` : Permet d'éteindre la LED du peripherique  
```#define GPIO_VALUE( ptr, gpio )``` : Permet de recuperer la valeur du peripherique d' entrée  
```delay ( half_period2 )``` : Permet d'attendre une periode donnée  

On  a compilé sur le PC puis copié l' executable sur le raspberry.  
Dans le makefile il y avait l'instruction "copy" pour envoyer l'exec vers le raspberry.



Séance 2:
==

###Création de module noyau: (ledp.c)

- créer le code du module
- le compiler à l'aide du makefile
- l'envoyer sur le raspi
- executer la commande pour l'insérer dans le noyau:
		sudo insmod ./ledbp.ko
- chercher dans le fichier /proc/devices le numéro major choisi par linux. 
- créer le noeud dans le répertoire /dev et le rendre accessible par tous. Le numéro mineur est 0 car il n'y a qu'une seule instance. 

sudo mknod /dev/ledbp c major 0
sudo chmod a+rw /dev/ledbp

####Afficher les messages de test de debogage:

echo "rien" > /dev/ledbp  
dd bs=1 count=1 < /dev/ledbp  
dmesg  
lsmod  

Séance 3:
==

###Pilotage d'un écran LCD en mode utilisateur et par un driver

> **Commentaire:**  
> Générer signal E   

```
void lcd_strobe()
{
	gpio_update(E, 1);  
	usleep(1);	//enable pulse width 	>= 300ns  
	gpio_update(E, 0);  
	usleep(1);  
}  
```

---  
> **Commentaire:**  
> Le gpio du raspi ne possède que 4 pins et non 8  
> Il faut donc envoyer 4 bits de poid fort,  
> puis les 4 derniers bits de poids faible  

```
void lcd_write4bits(int data)  
{  
  /* first 4 bits */
  gpio_update(D7, BIT(7,data)); 
  gpio_update(D6, BIT(6,data)); 
  gpio_update(D5, BIT(5,data)); 
  gpio_update(D4, BIT(4,data));  
  lcd_strobe();
  
  /* second 4 bits */
  gpio_update(D7, BIT(3,data)); 
  gpio_update(D6, BIT(2,data)); 
  gpio_update(D5, BIT(1,data)); 
  gpio_update(D4, BIT(0,data));  
  lcd_strobe();
}
```

---  
> **Commentaire:**  
> envoyer une instruction: RS = 0

```
void lcd_command(int cmd)
{
  gpio_update(RS, 0);
  lcd_write4bits(cmd);
  usleep(2000); 
}
```

---  
> **Commentaire:**  
> envoyer une donnée: RS = 1

```
void lcd_data(int character)
{
  gpio_update(RS, 1);
  lcd_write4bits(character);
}
```

---  
> **Commentaire:**  
> initialization

```
void lcd_init()
{
  gpio_update(E, 0);
  lcd_command(HEX(0011,0011)); /* initialization */
  lcd_command(HEX(0011,0010)); /* initialization */
  lcd_command(LCD_FUNCTIONSET    | LCD_FS_4BITMODE | LCD_FS_2LINE | LCD_FS_5x8DOTS );
  lcd_command(LCD_DISPLAYCONTROL | LCD_DC_DISPLAYON  | LCD_DC_CURSOROFF |  LCD_DC_BLINKOFF);
  lcd_command(LCD_ENTRYMODESET   | LCD_EM_LEFT | LCD_EM_DISPLAYNOSHIFT );
}
```

Séance 4:
==








