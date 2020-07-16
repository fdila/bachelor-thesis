# Discorso presentazione

## 1 (intro)
Buongiorno a tutti, sono Federica Di Lauro e ho svolto lo stage e la tesi presso il laboratorio IRALAB.
Il titolo della tesi è "sistema di controllo per base robotica outdoor"

## 2 (Obiettivi)
In IRALab era presente una base robotica a cui mancavano componenti e che necessitava di un nuovo sistema di controllo.

Lo scopo del mio stage è stato rimettere in sesto la base robotica, sia dal punto di vista hardware, che dal punto del sistema di controllo software. Inoltre questo sistema di controllo doveva essere integrato con ROS, un framework utilizzato nella robotica.

## 3 (Infrastruttura hardware)
Inizialmente era presente solo lo chassis con motori ed encoder, dei sensori che sono in grado di rilevare i giri del motore.
Come prima cosa è stato necessario cercare ed installare l'hardware mancante, ovvero:
- il microcontrollore che eseguirà il software per il sistema di controllo: è stata scelta una scheda nucleo, basata su un processore Arm
- un ponte H: è un dispositivo che permette di controllare la tensione erogata ai motori (e quindi la loro velocità e direzione di movimento) attraverso un segnale digitale generato dal microcontrollore.
- un convertitore usb-seriale per permettere la comunicazione tra microcontrollore  e computer.

## 4 (progettazione sistema)
Sul micro viene effettuata la lettura degli encoder, e vengono ricavate la velocità lineare ed angolare del robot. Viene eseguito il controllo in retroazione dei motori. 
Periodicamente vengono inviate le informazioni relative alle velocità, ed ogni volta che viene ricevuto un messaggio si aggiornano i setpoint dei controllori PID.

Sul computer viene calcolata l'odometria e si fanno cose


## 5 (progettazione software microcontrollore)
Lo sviluppo sul microcontrollore è stato bare-metal, ovvero non è stato utilizzato un sistema operativo. Questa scelta è stata effettuata non introdurre overhead e semplificare lo sviluppo del software.
I task individuati per il corretto funzionamento sono:
- lettura degli encoder, elaborazione dei dati per ricavare velocità lineare e angolare del robot e successivo invio al computer, da effettuare periodicamente
- Loop di controllo PID per la gestione dei motori, da effettuare periodicamente
- Ricezione di comandi da parte del computer.

Per gestire i due task periodici si è scelto di utilizzare due timer che generano un interrupt (rispettivamente a 10Hz per il primo e 100Hz per il secondo). I task sono gestiti nelle callback di questi interrupt.

Per la ricezione dei comandi dal computer abbiamo che viene generato un interrupt dal DMA ogni volta che vengono ricevuti n byte (dove n è la lunghezza del comando inviato dal pc). Nella callback di questo interrupt viene letto il messaggio e vengono aggiornati i setpoint per il controllo dei motori.

## 6 (encoder)
Abbiamo due encoder, uno per motore. Questi sensori generano due onde quadre, come si può vedere in figura. Per ogni giro di motore vengono generate un certo numero di pulsazioni, e si può anche sapere in quale direzione sta girando il motore, come possiamo vedere in figura: se il fronte di salita del segnale A precede quello di B sappiamo che il motore sta girando in senso orario e viceversa. 
La lettura di ogni encoder è stata effettuata tramite un timer, utilizzati in una particolare modalità specifica per gli encoder: i due output dell'encoder sono collegati a due canali del timer e un registro counter viene incrementato o decrementato automaticamente in base ai segnali dell'encoder.
Leggendo periodicamente questo registro possiamo stabilire quanto ha girato il motore in n millisecondi e da qui ricavare velocità lineare e velocità angolare del robot.

## 7 (ponte h)
Per poter controllre i motori è necessario introdurre un componente aggiuntivo, il ponte h, in quanto il microcontrollore non è in grado di erogare tensione e corrente sufficienti per guidare direttamente i motori. I motori girano in base alla tensione erogata, e in base alla polarità della tensione possono girare in senso orario o antiorario. Il ponte H è stato usato in modalità sign-magnitude, ovvero prende in input dal micro un segnale (alto/basso) per controllare la direzione del motore e un particolare tipo di segnale per regolare la tensione. Questo segnale è chiamato PWM, viene generato attraverso l'utilizzo di un timer, e permette di modulare la tensione media, e quindi di regolare la velocità del motore.
Per ogni motore abbiamo un segnale di direzione e un segnale PWM.

## 9 (pid)
Per poter effettuare un controllo stabile dei motori introduciamo un sistema in retroazione molto famoso, i controllori PID.
Abbiamo setpoint che è il valore che vogliamo far raggiungere al sistema (nel nostro caso la velocità del motore) e leggiamo il valore effettivo di velocità: la differenza tra i due è l'errore.
Il controllore PID opera in 3 parti:
- una parte proporzionale, in cui l'errore viene moltiplicato per una costante kp
- una parte integrativa in cui l'errore viene integrato e poi moltiplicato per una costante ki
- una parte derivativa in cui l'errore viene derivato e poi moltiplicato per una costante kd
Queste tre parti vengono poi sommate e il risultato sarà l'input dato al ponte H.

Il nostro sistema di controllo presenta 3 controllori PID: uno per il motore destro, uno per il motore sinistro e un controllore "cross-coupled" che opera sulla differenza di velocità tra i motori. Questo migliora il controllo del robot riducendo l'errore sull'orientamento dello stesso, in quanto i controllori singoli non sono completamente precisi.

## 7 (comunicazione)
Lato microcontrollore è stata utilizzata una periferica di tipo UART per la comunicazione seriale, utilizzata in modalità Direct Memory Access: in questo modo è sufficiente impostare settare la periferica che poi eseguirà le operazioni di I/O senza l'intervento della cpu. È stato inoltre utilizzato l'hardware flow control integrato nell'uart: in questo modo possiamo segnalare attraverso un apposito pin se la periferica uart è pronta per ricevere nuovi byte.
L'invio di informazioni avviene in modo periodico, mentre per la ricezione il DMA genera un interrupt dopo aver ricevuto un determinato numero di byte (corrispondente alla lunghezza del messaggio).
I dati, per poter garantire l'indipendenza da particolari architetture, sono serializzati/deserializzati attraverso la libreria nanopb, un'implementazione specifica per microcontrollori della libreria Protocol buffer.

Lato computer è stato utilizzato un modulo usb-seriale per potersi interfacciare con il microcontrollore.
Il software è stato scritto in python ed è stata utilizzata la libreria pyserial per poter utilizzare facilmente il modulo FTDI. Anche qui è stata utilizzata la libreria protocol buffers per la serializzazione dei dati.

## 8 (nodo ROS)
Il software python utilizza la libreria rospy per l'integrazione con ros. Ros è un framework che permette di scambiare messaggi attraverso il pattern publisher-subscriber. I nodi ROS possono essere sia publisher che subscriber: i publisher pubblicano i messaggi sui topic, i subscriber restano in ascolto sui topic ed effettuano una callback ogni volta che viene pubblicato un messaggio.

Il nodo sviluppato, serial_bridge, ha il compito di calcolare l'odometria, ovvero la stima della pose del robot, ogni volta che riceve un messaggio dal microcontrollore, e deve pubblicare le informazioni calcolate sui topic /odom e /tf.
È inoltre sottoscritto al topic /cmd_vel, dove vengono pubblicati i comandi di velocità che il robot deve eseguire: ogni volta che viene effettuata la callback il nodo serialbridge si occupa di serializzare il comando e di inviarlo al microcontrollore.

## 9 (testing) ~1 min
Per testare il sistema è stato sviluppato un nodo ROS per poter pubblicare sul topic /cmd_vel utilizzando un joypad, permettendo quindi di guidare manualmente il robot.
Grazie ad Rviz, uno strumento integrato in ROS, è stato inoltre possibile visualizzare l'odometria calcolata e verificarne la correttezza.

(far vedere la demo)

In conclusione possiamo dire che l'obiettivo dello stage è stato raggiunto con buoni risultati e il robot potrà essere utilizzato nel laboratorio per la ricerca.

Grazie per l'attenzione.