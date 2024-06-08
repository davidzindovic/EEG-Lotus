Navodila za uporabo:

!!POMEMBNO!! Če želiš meriti: Po prižigu naprave, poveži Mindwave Mobile 2 (naj ne bo na glavi takrat) tako, da ga prižgeš. Ko vidiš na HC-05, da sta se povezala (počasno utripa 2krat na 2 sekundi), izklopi Mindwave Mobile 2 (po prvih dveh utripih lučk po povezavi - povezana sta ko nehajo LED hitro utripati na HC-05). Počakaj, da se odpovežeta (hitro utripanje LED pomeni, da nista povezana), nato počakaj še cca 1 minuto ali več in ju poveži nazaj.
P.s.: obstaja možnost, da se bosta že prvič povezala in uskladila. Načeloma po tem, ko se enkrat uskladita se bosta vsakič, vendar je bolj priporočeno uporabiti zgornjo metodo.
P.S.S.: zgornji postopek je potrebno ponoviti tudi če ste prej uspešno povezali napravo, vendar ste Mindwave Mobile 2 disconnectali in nekaj časa (nekaj minut) neuporabljali.
P.S.S.: Mindwave mobile 2 poveži (in odpoveži ter ponovno poveži - zgoraj napisan postopek) preden vzameš plato oz. dokler se zadeva home-a.

Prvo je potrebno konfigurirati HC-05 modul za MindWave Mobile 2 napravo. Torej s pomočjo AT ukazov in dveh vezav skonfiguriramo modul in spremenimo utripanje lučk, ki je skladno s povezavo med modulom in EEG merilnikom.
--> najbolje opiše to ta članek: https://www.instructables.com/Brainwaves-Fly-a-Drone/

Ko je to storjeno, je najbolje namestiti knjižnico "Mindwave master", ki je pripeta v repozitoriju (dobljeno iz: https://github.com/orgicus/Mindwave/tree/master ). Zaženemo "Merjenje vrednosti" ali Merjenje vrednosti s software serial (potrebno prilagoditi vezavo).

Strojna oprema: Arduino UNO, pri čemer povežemo HC-05 Tx na Arduino UNO pin 8 (Software Serial), poleg napajanja.

Ostali programi sestavljeni s pomočjo omenjene knjižnice so orientirani bolj funkcijsko, saj drugače stvar preprosto ne deluje (glej primere).
