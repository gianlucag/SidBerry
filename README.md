Un SID jukebox realizzato con RasberryPi e il SID chip originale 6581. Il chip è alloggiato in una board custom collegata direttamente ai gpio del Rasberry!



La board può riprodurre qualsiasi SID file, basta caricare i file .sid nella sdcard del Rasberry, avviare il player e collegare all'uscita jack un paio di casse preamplificate o meglio uno stereo.

SID chip

Il SID originale, nelle varianti 6581 e 8580, è controllato caricando i suoi 29 registri interni con i valori opportuni al momento opportuno. La sequenza di byte inviati genera l'effetto o la musica desiderati. Ogni registro è 1 byte ed i registri sono in tutto 29 quindi c'è bisogno di almeno 5 gpio di indirizzo (2^5=32) e 8 gpio di dati per un totale di 13 gpio. Un altro gpio è richiesto per la linea CS del chip (Chip Select).



Fortunatamente il Raspberry ha ben 17 gpio e i 14 richiesti sono perfettamente pilotabili con la libreria WiringPi. Gli altri pin del chip servono per i due condensatori dei filtri interni (CAP_a e CAP_b), alimentazione, linea R/W (che andremo a collegare direttamente a GND, siamo sempre in scrittura), entrata del segnale di clock, linea di reset (collegata sempre a VCC) e uscita audio analogica (AUDIO OUT).

Ecco la configurazione dei registri interni del SID:



Qui per una descrizione più dettagliata del funzionamento interno del chip:

http://www.waitingforfriday.com/index.php/Commodore_SID_6581_Datasheet

Hardware

La board riproduce esattamente le "condizioni al contorno" per il SID chip come se si trovasse alloggiato in un Commodore 64 originale. L'application note originale mostra chiaramente i collegamenti da effettuare e i pochi componenti esterni richiesti (generatore di clock, condensatori e poco altro).



Unica differenza, le linee di indirizzo e dati vengono dirottate direttamente sui gpio del RasberryPi.

NOTA IMPORTANTE!: Il RasberryPi ragiona in logica CMOS a 3.3V mentre il SID chip è TTL a 5V quindi completamente incompatibili a livello di tensioni. Fortunatamente, siccome andiamo unicamente a scrivere nei registri, il RasberryPi dovrà soltanto applicare 3.3v ai capi del chip, più che sufficienti per essere interpretati come livello logico alto dal SID.



Lo schematico completo:



Software

Il grosso del lavoro. Volevo una soluzione completamente stand-alone, senza l'ausilio di player esterni (come ACID64) e quindi ho realizzato un player che emula gran parte di un C64 originale. L'emulatore è necessario in quanto i file .sid sono programmi in linguaggio macchina 6502 e come tali devono essere eseguiti. Il player è scritto in C/C++ e basato sul mio emulatore MOS CPU 6502   più un semplice array di 65535 byte come memoria RAM (il C64 originale ha infatti 64K di RAM). Il player carica il codice programma contenuto nel file .sid nella RAM virtuale più un codice assembly aggiuntivo che ho chiamato micro-player: sostanzialmente si tratta di un programma minimale scritta in linguaggio macchina per CPU 6502 che assolve a due compiti specifici:

installare i vettori di reset e interrupt alle locazioni corrette
chiamare la play routine del codice sid ad ogni interrupt
Questo perchè il codice che genera la musica in un comune C64 è una funzione chiamata ad intervalli regolari (50 volte al secondo, 50Hz). La chiamata è effettuata per mezzo di un interrupt esterno. In questo modo è possibile avere musica e un qualsiasi altro programma (videogioco ad esempio) in esecuzione nello stesso momento.

Altre componenti sono il parser per i file .sid e la libreria WiringPi per pilotare i GPIO del RaspberryPi.

Questo il layout dell'intero applicativo, a destra è la memoria RAM virtuale 64K



Questo è il codice assembler del micro-player:

[code lang="C"]
// istallazione del vettore di reset (0x0000)
memory[0xFFFD] = 0x00;
memory[0xFFFC] = 0x00;</p>
// istallazione del vettore di interrupt, punta alla play routine (0x0013)
memory[0xFFFF] = 0x00;
memory[0xFFFE] = 0x13;

// codice del micro-player
memory[0x0000] = 0xA9;
memory[0x0001] = atoi(argv[2]); // il registro A viene caricato con il numero della traccia (0-16)

memory[0x0002] = 0x20; // salta alla init-routine nel codice sid
memory[0x0003] = init &amp; 0xFF; // lo addr
memory[0x0004] = (init &gt;&gt; 8) &amp; 0xFF; // hi addr

memory[0x0005] = 0x58; // abilitiamo gli interrupt
memory[0x0006] = 0xEA; // nop
memory[0x0007] = 0x4C; // loop infinito
memory[0x0008] = 0x06;
memory[0x0009] = 0x00;

// Interrupt service routine (ISR)
memory[0x0013] = 0xEA; // nop
memory[0x0014] = 0xEA; // nop
memory[0x0015] = 0xEA; // nop
memory[0x0016] = 0x20; // saltiamo alla play routine
memory[0x0017] = play &amp; 0xFF;
memory[0x0018] = (play &gt;&gt; 8) &amp; 0xFF;
memory[0x0019] = 0xEA; // nop
memory[0x001A] = 0x40; // return from interrupt
[/code]

