Navodila za uporabo:

!!POMEMBNO!! Po prižigu naprave, poveži Mindwave Mobile 2 (naj ne bo na glavi takrat) tako, da ga prižgeš. Ko vidiš na HC-05, da sta se povezala, izklopi Mindwave Mobile 2 (po prvem utripu lučk po povezavi). Počakaj, da se odpovežeta (hitro utripanje LED pomeni da nista povezana), nato počakaj še cca 1 minuto ali več in ju poveži nazaj.
P.s.: obstaja možnost, da se bosta že prvič povezala in uskladila. Načeloma po tem, ko se enkrat uskladita se bosta vsakič.

Prvo je potrebno konfigurirati HC-05 modul glede na MindWave Mobile 2 napravo. Torej s pomočjo AT ukazov in dveh vezav skonfiguriramo modul in spremenimo utripanje lučk, ki je skladno s povezavo med modulom in EEG merilnikom.
--> najbolje opiše to ta članek: https://www.instructables.com/Brainwaves-Fly-a-Drone/

Ko je to storjeno, je najbolje namestiti knjižnico "Mindwave master", ki je pripeta v repozitoriju (dobljeno iz: https://github.com/orgicus/Mindwave/tree/master ). Zaženemo "Merjenje vrednosti".

Strojna oprema: Arduino UNO, pri čemer povežemo HC-05 Tx na Arduino UNO Rx (pin 0), poleg napajanja.

Ostali programi sestavljeni s pomočjo omenjene knjižnice so orientirani bolj funkcijsko, saj drugače stvar preprosto ne deluje (glej primere)
