// -*- c-basic-offset: 4; c-backslash-column: 79; indent-tabs-mode: nil -*-
// vim:sw=4 ts=4 sts=4 expandtab
/* Copyright 2012
 * This file is part of Fachoda.
 *
 * Fachoda is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Fachoda is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Fachoda.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "proto.h"

struct prime prime[NBPRIMES];
struct village village[NBVILLAGES];

#define NBRADIO 1
char *nomvillage[NBVILLAGES] = { "Mokolo", "Badagadir", "Mokassaville", "Miditana", "Zawabi", "Bogomips", "Osk", "Homene", "Joytown", "Peacetown" };
char msgactu[1000];
int msgactutime=0;
int campactu;
#define NBMSG 2
char *villagemsg[NBMSG][4][2] = {
    {
        {"Le president Francais pense qu'un genocide dans le village de %s ne serait pas tres important.",
         "The President of France think that a genocide in the village of %s wouldn't be of much importance"},
        {"Les habitants de %s refusent de payer leurs contributions de guerre ! Que dieux s'abatte sur eux !",
         "%s' residents refuse to pay a fee for the war ! Shall the Lord crush their heads !"},
        {"Il faut defendre les investissements occidentaux dans ce pays ! Des subversifs se terrent a %s. Quelques bombes devraient apprendrent les bienfaits de la civilisation a ces barbares !",
         "We have to defend occidental investments in this country. Some subversives bury themselves in the region of %s. Some few bombs should teach to those barbarians the goodness of civilisation."},
        {"Le journal local de %s critique notre transition vers la democracie ! Bombardez la population pour qu'elle se ressaisisse !",
         "The local newspaper of %s critisizes our transition to democracy ! Bomb the population to pull itself together !"}
    } ,
    {
        {"Une imprimerie clandestine de l'opposition doit se trouver a %s. Detruisez la par inadvertance. Poussez l'inadvertance jusqu'a rasez tout le village s'il le faut.",
         "The oposition should own an underground printing-plant in %s. Destroy it inadvertently. So inadvertently that the village be leveled to the ground if you must."},
        {"Il reste des fideles a Mokassa caches dans le village de %s ! Il faut aneantir ces malfaisants quitte a raser le village !",
         "Amongst the population of %s remain people who are still in favour of Mokassa ! You MUST annihilate them ! Wipe out all the village if you wish !"},
        {"British-Betoneum a decroche la reconstruction de %s ! Si ce village etait entierement detruit, ce contrat prendrait une immense valeure !",
         "British-Concrete win the first prize for the reconstruction of %s ! How fabulous would worth this contract if this village was reduced to nothing..."},
        {"Des terroristes Almouches prennent la population de %s en otage ! Delogez les terroristes !",
         "Almouche's terrorists are maintaining the population of %s in captivity ! Clear out the place !"}
    }
};
char *botavionmsg[2][NBMSG][4][2] = {
    {
        {
            {"Un diplomate survole la region dans un %s. S'il lui arrivait un accident, nous pourrions mettre son deces sur le compte de l'oposition. Pourriez vous vous rendre vers %.0f, %.0f afin d'etudier la question ?",
             "A diplomat is flying over the country in a %s. If he had an accident, we could charge the oposition with his death. Could you go to %.0f %.0f %.0f to study the question ?"},
            {"Le %s qui survole la zone vers %.0f %.0f vient de photographier nos agents...",
             "The %s which fly over the country around %.0f %.0f has just taken pictures of our agents..."},
            {"Le %s qui survole la zone vers %.0f, %.0f vient de photographier nos SAS ! Abattez le vite avant que ces photos n'aillent alimenter quelque presse a scandale !",
             "The %s which fly over the country around %.0f %.0f has just taken our SAS in pictures ! Shoot him down before those pictures cause scandal in the newspapers"},
            {"Un %s repere vers %.0f, %.0f s'apprete a larger des tracts appelant notre bon peuple a se soulever.",
             "A %s spoted around %.0f %.0f is about to drop propaganda leaflets calling our people to revolt."}
        },
        {
            {"Des investigateurs etrangers a la solde des bamatsis pourraient se trouver a bord du %s repere en %.0f, %.0f. Ils pourraient chercher a quitter le pays avec des documents nuisibles.",
             "Foreign investigators, paid by the bamatsies, could fly aboard the %s spoted at %.0f %.0f. They could be trying to leave the country with compromising documents."},
            {"Le %s impie detecte en %.0f %.0f aurait pour objectif de bombarder le grand temple de Dieu ! Detruisez l'infidele !",
             "The irreligious %s spoted at %.0f %.0f should approch a holly site with the secret aim to bomb it ! Destroy the infidel !"},
            {"Le %s detecte en %.0f %.0f est le modele concurent du notre pour cette gamme d'appareils. S'il etait abatu, cela prouverait indiscutablement que notre materiel est le meilleurs.",
             "The %s spoted at %.0f %.0f is the concurent pattern of ours for this type of fighter. If it were shot down, it would be proven that our war material is better."},
            {"Sur ordre personel de la fille du president, le %s situe en %.0f %.0f doit etre abatu.",
            "Upon personal order of the President's daughter, the %s flying at %.0f %.0f must be shot down."}
        }
    }, {
        {
            {"Habyakassa, le ministre de l'instruction populaire de Mokassa, se trouve dans un %s, repere vers %.0f %.0f. La mort de ce loyal ami du president, si elle surgissait maintenant, serait certainement vengee par son peuple et favoriserait notre entreprise",
             "Habyakassa, the Mokassa' minister for State education, is flying aboard the %s spoted around %.0f %.0f. This loyal friend of the president would certainly be revenged for his death, if by chance it happens now, which would help us in our plans."},
            {"Un de nos pilotes viens de blasphemer. Son %s se trouve en %.0f %.0f. Les doctes promettent l'acces au paradis au guerrier valeureux qui l'abbattra",
             "One of our pilot has just blasphemed. His %s is located at %.0f %.0f. The Learned priests promise paradise to the valorous warior who will shot him down."},
            {"Le %s qui s'agite vers %.0f %.0f est l'avion de demonstration d'une firme concurente ! Abbattez le ! Le marche du Sierra-Freedom est reserve.",
             "The %s that is bustling around %.0f %.0f is the demonstrative plane of a concurent company ! SHOT-HIM-DOWN-RIGHT-NOW ! Sierra-Freedom is out private market."},
            {"Un ancien pilote de notre glorieuse armee invincible s'apprete a trahir la cause sacree du peuple. Son %s, repere en %.0f %.0f, vol vers l'amerique ! Detruisez le afin que notre technologie ne tombe pas entre des mains imperialistes !",
             "A former pilot of our glorious and invincible army is about to betray the people for the side of our ennemies ! His %s, spoted around %.0f %.0f, is flying to america ! Shoot him down before that our technology fall into the imperialists' hands !"}
        }, {
            {"Le pilote du %s en %.0f %.0f est soupconne d'etre un sobel (soldat le jour, rebele la nuit). Nous aussi pouvons jouer double jeu : abbattez le !",
             "The pilot thats flying in the %s at %.0f %.0f is suspected t be a betrayer. We can play double games also : shot him down !"},
            {"Un journaliste etranger fait un reportage mal intentionne sur nos activitees. Il se trouve a bord du %s citue en %.0f %.0f. Il ne faudrait pas qu'il puisse continuer son reportage !",
             "A foreign reporter is working on our activites with bad intentions. He is taking some pictures aboard the %s located at %.0f %.0f. Too bad if he can finish his work..."},
            {"Un %s ancien modele a ete detecte en %.0f %.0f. S'il etait abatu, cela serait un argument de poid pour vendre notre nouvelle version...",
             "An old fashionned %s was spoted at %.0f %.0f. If it was destroyed, perhaps they would like to replace it by a new one ?"},
            {"Le maire de Moscow a lance ses reporter sur le terrain pour plonger le Kremlin dans l'embarras. Un de ces agents survole la zone dans un %s vers %.0f %.0f. A abbatre !",
            "The major of Moscow has sent his reporters on the ground to push the Kremlin into trouble. One of these agents in flying over the country in a %s towards %.0f %.0f. To be shot down."}
        }
    }
};
char *botvehicmsg[2][NBMSG][4][2] = {
    {
        {
            {"Le leader du Front des Patriotes Bamatsis se trouverait dans le %s, en %.0f %.0f",
             "The Chief of the Union of Bamatsis' patriots would be traveling in the %s at %.0f %.0f"},
            {"Un %s rode vers notre camp d'entrainement secret en %.0f %.0f !",
             "A marauding %s is approching our secret training camp at %.0f %.0f !"},
            {"Le %s repere vers %.0f %.0f est le prototype d'un nouveau char israelien. Si seulement cette mise a l'essai pouvait etre desastreuse...",
             "The %s spotted near %.0f %.0f is the pattern of a new israelian tank. We wish this training round was a desaster..."},
            {"Le chef des terroristes Almouches vient de se commander une pizza depuis le %s, en %.0f %.0f !",
             "The Chief of Almouches' terrorists has just ordering a pizza from the %s at %.0f %.0f !"}
        },{
            {"Le %s, vu en %.0f %.0f, s'approche d'une unite de CRAP qui se sont perdus dans la foret. Detruisez le avant qu'il ne les decouvre !",
             "The %s sean near %.0f %.0f is approching a CRAP squad lost in the woods. Destroy himt before he discover them"},
            {"L'equipage du %s situe en %.0f %.0f vient de massacrer une colonne de refugies Bamatsis ! Le Front des Patriotes Bamatsis ne devrait pas laisser impuni cet acte barbare !",
             "The tank-crew of the %s located at %.0f %.0f comes to butcher a refugee column. The Union of Bamatsis' patriots should not let this barbarous crime unpunished !"},
            {"Le %s qui se trouve en %.0f %.0f possede un nouveau type de blindage soit disant resistant a nos bombes. Ceci n'est pas tres serieux.",
             "The %s spotted at %.0f %.0f use a new kind of armour which is said to rerists to our bombs. Did they just wanted to kid us ?"},
            {"Le %s vu en %.0f %.0f est un char de modele allemand ! Depuis quand ces Almouches se permettent-ils d'acheter autrechose que ce qu'on leur vend au marche noir ?",
             "The %s sean near %.0f %.0f is made in Deutshland ! What gives Almouches the idea that they can buy something else that what WE sold us on black-market ?"}
        }
    },{
        {
            {"L'equipage du %s en %.0f %.0f fait preuve de mansuetude a l'egard des Bamatsis ! Que deviendraient nos projets si la haine tarrissait ?",
             "The tank-crew of the %s at %.0f %.0f is giving proof of mansuetude towards Bamatsis. What would become our plans if hate runs dry ?"},
            {"L'equipage du %s situe vers %.0f %.0f pose des questions à propos de l'origine de nos finances. Le doute n'est pas permis a ceux qui ont la foi !",
             "The tank-crew of the %s located at %.0f %.0f asks questions about where our funds originate from. Doubt is not allowed amongst the believers !"},
            {"Des imbéciles du contingent ont peint un slogan contre la famille royale sur leur %s ! Lachez quelques bombes vers %.0f %.0f pour faire leur education.",
             "Dumb heads from the intake have painted slogans against the Royal family on their %s ! Drop some bombs towards %.0f %.0f do make their education."},
            {"Il est urgent de redynamiser nos troupes qui ne rencontrent pas assez de resistance. Un tank devrait etre détruit pour rendre les autres plus combatifs. Le %s actuelement en %.0f %.0f ferait une bonne cible : l'equipage ne compte que des pouilleux originaires du Kamtchatka.",
             "It is urgent to push into action the squad which do not encounter enough resistance. A tank should be destroyed for having the others more pugnacious. The %s located at the present time at %.0f %.0f would be a good target : the crew count only amongst lousy persons from Kamtchatka, standing for nothing."}
        },{
            {"Le pilote du %s actuelement en %.0f %.0f a des amis Bamatsis ! Executez le traitre !",
             "The driver of the %s for now at %.0f %.0f count Bamatsis amongst his friends ! Terminate the traitor !"},
            {"L'equipage du %s qui fait route vers %.0f %.0f fait preuve de clemence a l'eguard des populations infideles. Punissez ces scelerats !",
             "The tank-crew of the %s twhich is driving towards %.0f %.0f gives proof of clemency towards the infidel populations. Punish the wickeds !"},
            {"Detruisez le %s situe en %.0f %.0f dont l'equipage reclame le payment des arrieres de solde...",
             "Destroy the %s located at %.0f %.0f for its crew is laying claim to draw the owed pay..."},
            {"L'equipage du %s actuelement en %.0f %.0f s'est oppose au viol de jeunes femmes Almouches !? Peut etre est-ce le debut de la demoralisation des troupes ? Remoralisons les a coup de bombes !",
             "The tank-crew of the %s now at %.0f %.0f was opposed to the young Almouche wemens' rape !? Perhaps is it the begining of demoralization for these troops ? Let's remoralize them with some  few bombs !"}
        }
    }
};
void newprime() {
    int j, i, k;
    char botname[200];
    for (i=0; i<NBPRIMES && prime[i].reward!=0; i++);
    if (i<NBPRIMES) {
        campactu=(campactu+1)&3;
        msgactutime=300;
        strcpy(msgactu,lang?"Try to make more collateral victims":"Essayez de faire davantage de dommages colateraux");
        switch ((int)(drand48()*3.)) {
        case 0:
            // détruire une maison d'un village
            k=0;
            do {
                k++;
                j=NBVILLAGES*drand48();
                prime[i].no=village[j].o1+(village[j].o2-village[j].o1)*drand48();
            } while (k<10 && obj[prime[i].no].type==DECO);
            if (k<10) {
                sprintf(msgactu,villagemsg[(int)(drand48()*NBMSG)][campactu][lang],village[j].nom);
                prime[i].camp=campactu;
                prime[i].reward=1000+1000*drand48();
                prime[i].dt=-1;
                prime[i].endmsg=NULL;
            }
            break;
        case 1:
            // détruire un bot avion
            k=0;
            do {k++; j=NBBOT*drand48();} while (k<10 && bot[j].camp==-1);
            if (k<10) {
                if (j<NbHosts) {
                    strcpy(botname,plane_desc[bot[j].navion].name);
                    strcat(botname," (");
                    strcat(botname,&(playbotname[j])[0]);
                    strcat(botname,")");
                }
                sprintf(msgactu,botavionmsg[bot[j].camp==campactu][(int)(drand48()*NBMSG)][campactu][lang],j<NbHosts?botname:plane_desc[bot[j].navion].name,obj[bot[j].vion].pos.x,obj[bot[j].vion].pos.y);
                prime[i].camp=campactu;
                prime[i].reward=1000*drand48()+(bot[j].camp==campactu?4000:2000);
                prime[i].no=bot[j].vion;
                prime[i].dt=-1;
                prime[i].endmsg=NULL;
            }
            break;
        case 2:
            // détruire un bot véhic
            k=0;
            do {k++; j=NBTANKBOTS*drand48();} while (k<10 && vehic[j].camp==-1);
            if (k<10) {
                sprintf(msgactu,botvehicmsg[vehic[j].camp==campactu][(int)(drand48()*NBMSG)][campactu][lang],vehic[j].nom,obj[vehic[j].o1].pos.x,obj[vehic[j].o1].pos.y);
                prime[i].camp=campactu;
                prime[i].reward=1500+1000*drand48();
                prime[i].no=vehic[j].o1;
                prime[i].dt=-1;
                prime[i].endmsg=NULL;
            }
            break;
        }
    }
}
