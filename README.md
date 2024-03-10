Navodila za uporabo:

Prvo je potrebno konfigurirati HC-05 modul glede na MindWave Mobile 2 napravo. Torej s pomočjo AT ukazov in dveh vezav skonfiguriramo modul in spremenimo utripanje lučk, ki je skladno s povezavo med modulom in EEG merilnikom.
--> najbolje opiše to ta članek: https://www.instructables.com/Brainwaves-Fly-a-Drone/

Ko je to storjeno, je najbolje namestiti knjižnico "Mindwave master", ki je pripeta v repozitoriju (dobljeno iz: https://github.com/orgicus/Mindwave/tree/master ). Zaženemo "Merjenje vrednosti".

Strojna oprema: Arduino UNO, pri čemer povežemo HC-05 Tx na Arduino UNO Rx (pin 0), poleg napajanja.

Ostali programi sestavljeni s pomočjo omenjene knjižnice so orientirani bolj funkcijsko, saj drugače stvar preprosto ne deluje (glej primere)
