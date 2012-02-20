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
#include <jpeglib.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>
#include "proto.h"
#include "keycodesdef.h"
#include "sound.h"
#include "file.h"

pixel32 *presentimg;

int IMGX, IMGY;

void jloadpresent() {
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
//  djpeg_dest_ptr dest_mgr = NULL;
    FILE *input_file;
    JSAMPROW imgtmp;
//  dest_mgr = jinit_write_bmp(&cinfo, FALSE);
    cinfo.err = jpeg_std_error(&jerr);
    jpeg_create_decompress(&cinfo);
    if ((input_file = file_open("complex.jpg", DATADIR, "r")) == NULL) {
        exit(-1);
    }
    jpeg_stdio_src(&cinfo, input_file);
    jpeg_read_header(&cinfo, TRUE);
    jpeg_start_decompress(&cinfo);
    IMGX=cinfo.output_width;
    IMGY=cinfo.output_height;
    if ((presentimg=(pixel32*)malloc((IMGX+1)*(IMGY+1)*sizeof(pixel32)))==NULL) {
        perror("malloc presentimg"); exit(-1);
    }
    imgtmp=(JSAMPROW)malloc(cinfo.output_width*3);
    while (cinfo.output_scanline < cinfo.output_height) {
        unsigned i;
        jpeg_read_scanlines(&cinfo, &imgtmp, 1);
        for (i=0; i<cinfo.output_width; i++) {
            presentimg[(cinfo.output_scanline-1)*IMGX+i].r=imgtmp[i*3];
            presentimg[(cinfo.output_scanline-1)*IMGX+i].g=imgtmp[i*3+1];
            presentimg[(cinfo.output_scanline-1)*IMGX+i].b=imgtmp[i*3+2];
        }
    }
    free(imgtmp);
    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);
    fclose(input_file);
}
/*
void darkpresent() {
    int x,y;
    for (y=0; y<IMGY; y++) {
        for (x=0; x<IMGX; x++) {
            if (x==IMGX-1) presentimg[y*IMGX+x].r=presentimg[y*IMGX+x].g=presentimg[y*IMGX+x].b=128;
            else presentimg[y*IMGX+x].r=presentimg[y*IMGX+x].g=presentimg[y*IMGX+x].b=(((presentimg[y*IMGX+x].r+presentimg[y*IMGX+x].g+presentimg[y*IMGX+x].b)-(presentimg[y*IMGX+x+1].r+presentimg[y*IMGX+x+1].g+presentimg[y*IMGX+x+1].b))>>2)+128;
        }
    }
}
*/
void affpresent(int dx,int dy) {
    int y;
    int xb=((SX-IMGX)>>1)+dx, yb=((SY-IMGY)>>1)+dy, clipx1=0, clipx2=0;
    if (xb+IMGX>SX) clipx2=xb+IMGX-SX;
    if (xb<0) { clipx1=-xb; xb=0; }
    MMXMemSetInt((int*)videobuffer,BACKCOLOR,SX*SY);
    for (y=0; y<IMGY && y+yb<SY; y++)
        if (y+yb>=0)
            MMXCopy((int*)videobuffer+(y+yb)*SX+xb,(int*)presentimg+y*IMGX+clipx1,IMGX-clipx1-clipx2);
}
void affpresentanim(int d) {
    int y;
    int xb=(SX-IMGX)>>1, yb=(SY-IMGY)>>1, clipx=0;
    if (xb<0) { clipx=-xb; xb=0; }
    for (y=0; y<IMGY; y++) {
        int yd;
        if (y&1) {
            yd=y+d*drand48();
            if (yd>=IMGY) yd=IMGY-1;
        } else {
            yd=y-d*drand48();
            if (yd<1) yd=1;
        }
        MMXCopy((int*)videobuffer+(y+yb)*SX+xb,(int*)presentimg+yd*IMGX+clipx,IMGX-clipx-clipx);
    }
}

void animpresent() {
//  int dt1=0,dt2=0,dt3=0;
//  int m1=-1,m2=-1,m3=-1, om;
    int d=20;
/*  struct timeval GTime;
    long DT;
    #define  NBAM 19
    char *(mesg[NBAM])={
        "Code & Gfx by RiXed",
        "(c) Cedric Cellier, feb 2000",
        "Anarchie en Chiraquie !",
        "Any ressemblance with actual facts...",
        "Visit http://rixed.free.fr",
        "Click somewhere, try to get me !",
        "Now, click on the window.",
        "Fear the Moshito !",
        "Fly like a real penguin !",
        "This is still in development",
        "A game for true patriots",
        "The complex explains the whole thing",
        "Are pinguins still complexed ?",
        "Ah Dieux ! Que la guerre est jolie !",
        "Tanks can be destroyed with guns",
        "use option -l to play at night",
        "Complain to rixed@free.fr",
        "How high can a penguin fly ?",
        "Wreak vengeance on Fachoda !"
    };*/
    TextClipX1=(SX-IMGX)/2;
    TextClipX2=(SX-IMGX)/2+250;
    TextColfont=0xD0D0D0;
    jloadpresent();
    MMXMemSetInt((int*)videobuffer,*(int*)(presentimg+IMGX+1),SX*SY);
//  gettimeofday(&GTime,NULL);
    playsound(VOICEEXTER, PRESENT, 1., &voices_in_my_head, true, false);
    while (d) {
        affpresentanim(d);
        d--;
        buffer2video();
        xproceed();
    };
    TextClipX1=TextClipX2=0; TextColfont=0;
}

int colcamp[4]={0xFFD090,0x70FF70,0x80D0FF,0xFFFF70};

char *scenar[4][4][2] = {
    {
        {"La Republique Democratique et Independante des Trois Villages Unifies est un petit pays, que preside le Marechal a vie Senechal Hyppolite Gedeon Mokassa. Apres son accession a l'independance, la R.D.I.T.V.U. a pu concerver de bons rapports avec la France ce qui lui permis d'ecouler une encombrante production petroliere. Un exemple de cooperation en verite.",
         "The Republique Democratique et Independante des Trois Villages Unifies is a little country, the destiny of wich being presided over for ever by Senechal Hyppolite Gedeon Mokassa. After its accession to independance, the R.D.I.T.V.U could keep so good relations with France than it was able to dispose of its cumbersome petrol production. A true cooperation, actualy."},
        {"Mais les mechants Bamatsis, l'ethnie minoritaire, veulent saper cette amitie reelle avec la France en s'en prenant a la famille de notre bon president a vie Mokassa. Helas pour eux, les miliciens du president ainsi que son peuple fidele ont etes prepares depuis de longues annees, par un travail mediatique meticuleux, a hair les Bamatsis, et ils sont determines a exterminer jusqu'au dernier ces chiens et leurs complices de l'opposition.",
         "But the sly Bamatsees, the minority ethnic group, want to undermine this actual friendship with France by finding fault with the good president's clan. So bad for them, president's milicians alltogether with his fidel people were prepared for years by a meticulous mediatic work to hate the Bamatsees, and they are determined to anihilate untill the last one those pigs and their accessories in the opposition."},
        {"Vous etes Drazsyck Totonovick et avez ete repere a Srebrenica par un agent recruteur du 1er RPIM, l'ancienne coloniale, alors que vous serviez dans la milice serbe. Vous etes en poste dans la capitale de la Republique en tant qu'instructeur-formateur de la police politique de notre ami le president Mokassa, lorsque parvient la nouvelle que du Bas-Wanana voisin se prepare une offensive du Front des Patriotes Bamatsis en exil, orchestree depuis Washington pour destabiliser la region afin de remettre en question l'attribution des marches petroliers.",
         "You are Drazsyck Totonovick and have been noticed in Srebrenica by a 1st RPIM' (the former colonial army) recruiting sergent, while you served in the serb militia. You were in duties at the main town of the Republic as an instructor for the political police of our friend president Mokassa, when came the news according to wich in the bordering Low-Wanana an offensive of the Exiled Bamatsees Patriots' Front was prepared by Washington to destabilize the zone and recall in question the allotment of petrol market."},
        {"Contre les pretentions hegemoniques americaines, defendons la Francophonie et l'amitie France-Afrique ! Pour une Republique Democratique des Trois Villages Unifies libre et independante, longue vie au president marechal a vie Senechal Hyppolite Gedeon Mokassa ! Vive l'Afrique ! Vive la France ! Morts aux Bamatsis !",
         "Against the american claim to hegemony, let us defend the community of French-speaking peoples and the France-Africa friendship ! For a free and independant Republique Democratique des Trois Villages Unifies, up with President for life Senechal Hyppolite Gedeon Mokassa ! Hurrah for Africa ! Hurrah for France ! Death to the Bamatsees !"},
    }, {
        {"Jamais le marche du petrole de la Republique des Trois Villages Unifies ne furent renegocies depuis que le despotte Mokassa fut mis au pouvoir par la France. En retour, le tyran attribue le monopole de l'exploitation du petrole de la Republique des Trois Villages Unifies aux thrusts petroliers Francais. Cette terrible dictature insupportable, vis à vis des droits de l'homme, doit cesser ! Maintenant que suite a la fin de la guerre froide le risque de destabilisation est enraye, il faut renverser le dictateur sanguinaire pour le remplacer par le president democratiquement elu de notre choix.",
         "The petrol market in the Republique des Trois Villages Unifies has never been negotiate since president-despot Mokassa was given power by France. The tyrant alloted the whole extraction of Republic des Trois Villages' petrol to the French fiul trust. This unbearable dictatorship must cease for human rights's sake ! Now that the cold war and the risk of destabilization by the russians are over, we must overthrow the bloodthirsty dictator and replace him by the democraticaly elected president of our choice." },
        {"Une guerilla dont les chefs sont acquis a la cause de Washington puisque le CIA les forme, les arme et finance leurs ecoles religieuses, va bientot se deverser, depuis le Bas-Wanana sur la Republique des Trois Villages Unifies, et balayer le regime tyranique de Mokassa qui ne tient que grace au soutient militaire de la France. L'heure est venue de lancer l'offensive afin de renegocier le marche des droits de l'hommes dans la region !",
        "The guerilla of the Bamatsees, which chiefs are gained to Washington's cause because CIA train, fund, and supply them with weapons and religious schools, is going to pour into the Republique des Trois Villages Unifies from the Low-Wanana land. It will for sure sweep away the tyranic rule of Mokassa, that is only supported by the French army and by no way by the people. Time is come to launch our rebels so that we can renegociate the human rights's market in this country !"},
        {"Vous etes Jim Parano, officier de la Protect & Defend the Innocents, Inc., une officine militaire privee a caractere humanitaire. Vous avez ete envoye au Bas-Whanana dans le cadre de l'operation 'Thank-you Bill' pour participer aux manoeuvres de guerrilla deguise en guerrier tribal bamatsi.",
        "You are Jim Parano, an officer of the Protect & Defend the Innocents, Inc., a private military unit concerned by humanitary considerations. You have been sent to Low-Wanana in the setting of operation 'Thank-you Bill' to take part to the guerilla's drill, disguised like a tribal bamatsee warior."},
        {"Decollez immediatement, prenez part aux combats, nos awacs vous transmettrons les objectifs prioritaires.",
         "Take off at once and attend the battle. Our awacs will broadcast prioritary targets."}
    }, {
        {"Le Sierra-Freedom est un petit pays en proie a une agitation sociale provoquee par l'attachement de la population locale a des modes de vie archaiques, empechant l'Etat Sierrafreedomais d'appliquer la saine gestion conseillee par la banque mondiale. Si ce petit pays bascule dans l'anarchie, il ne pourra plus rembourser sa dette et donc ne sera plus considere avec confiance part la communaute internationale qui lui refusera de nouveaux credits et finalement, c'est la pauvre population du Sierra-Freedom qui en souffrira.",
         "Sierra-Freedom is a little coutry which is a prey to agitation caused by the natives who do not understand the necessity of the rational economical policy proned by the World-wide Bank. If this country dips into desorder and anarchy, it won't be able to settle the WMF any more, so it won't be considered as a thrustable country any more by the international community that will not accord any more loans to Sierra-Freedom, and finaly the poor people shall suffer from this situation."},
        {"Heureusement pour la democratie, La Grande Bretagne, qui possede des liens de cooperation privilegies avec le Sierra-Freedom, aide le regime en violant l'embargo sur les armes pour vendre a l'armee sierrafreedomaise les tanks, les avions et les bombes qui lui permettent de pacifier peu a peu le climat social.",
         "Happily for democracy, Great Britain, in the name of the priviledged cooperative links with this country, helps Sierra-Freedom's government by transgressing the weapons embargo and selling at the army of Sierra-freedom the tanks, planes and bombs, allowing it to slowly lower the social tenseness"},
        {"Vous etes Mickael Fireman, ancien directeur des ressources humaines du port de Liverpool reconverti en conseiller militaire, prete pour l'occasion au gouvernement Sierrafreedomais par la Grande Bretagne. Profitez de votre visa touriste pour aider l'armee sierrafreedomaise a faire bon usage du materiel que votre pays lui vend.",
         "You are Mickael Fireman, former chief of the personnel department for the docks of Liverpool, now having switch over to the new job of military advisor. You have been sent for the occasion to the sierrafreedom's government by the Great Britain. Turn your tourist's visa to account to help sierrafreedom's army to make a good use of the war-material that your counrty have just sold to them."},
        {"Decollez et attendez que votre employeur, le service après vente d'une firme exportatrice de gadgets, jouets et matériel de guerre, vous fasse parvenir d'avantage d'information par radio.",
         "Take off and wait for your employer, the technical assistance after sold of a compagny exporting gadgets, toys and war-material, radio you more up to date informations." }
    }, {
        {"Vous etes Alexandrovich Mafiov, membre de l'Etat major secret du maire de Moscou ainsi que proche de la petite coterie du Kremlin. Helas, les elections legislatives en Russie risquent de faire perdre la majorite a la Douma au clan du president, et avec elle le controle de quelques unes des plus grandes industries de Russie ! De meme, votre vieil ami et ancien camarade de KGB le liberal maire de Moscou a de fort interets dans la province Almouche qui risquent d'etre remis en question par le président de la province, parain de la mafia locale.",
         "You are Alexandrovich Mafiov, member of the secret head-quarters of Moscow major, and close to Kremlin's coterie. Halas, parliamentary elections in Russia endanger the president side which still rule Douma untill then. The control of some of the most important companies of Russia shall be lost if the elections are lost. At the same time you have your old friend and former KGB partner the liberal major of Moscow whom biggest interests are in the province of Almouchy and are recalled in question by the president of Almouchy, also the leader of the local mafia."},
        {"Les hommes de main du chef du Kremlin et du maire de Moscou se sont donc retrouves pour perpetrer quelques attentats visant la population des ghettos moscovites, afin d'en accuser un groupuscule terroriste almouche, fournissant un pretexte pour une douloureuse mais necessaire operation de restauration de l'ordre dans la province.",
         "So, the men-of-all-trades of the Kremlin's chief and of the major of Moscow worked together to perpetrate some attempts upon the population of the ghettos around moscow, so that they can charge an almost unknown almouchian terrorists group for the crime and start operations to restore law and order in Almouchy."},
        {"Vous avez etes charge, d'une part de mener une guerre sans repit a l'affreuse population terroriste almouche, afin que les electeurs et l'opposition remarquent bien la fermete avec laquelle le gouvernement reprime les ennemis de la patrie, et d'autre part de veiller a la destitution du regime Almouche et a son remplacement par un gouvernement provisoire, afin de favoriser la transition de la russie vers la democracie et le progres economique et social.",
         "You were charged with the direction of a war without respite against the hideous almouchian people, so that electors and political opposition notice well the hardness whith wich the government punish its foes. You must also help the destitution of the almouchian government, which will help the Russia's transition to democracy and economical and social progress."},
        {"Vive la mere patrie eternelle ! En avant glorieuse armee russe !",
         "Long life to mother native-country, everlasting Russia ! Go ahead, glorious russian army !"}
    }
};
/*
int presentold() {
    int i=0, phaz=5, phaztxt=0;
    darkpresent();
    do {
        affpresent();
        switch(phaz) {
        case 0:
            TextClipX1=10;
            TextClipX2=_DX-10;
            pstr("Sierra Freedom",10,0x80D0FF);
            pstr(!lang?"Volez dans la peau d'un envoye special intrepide au service de sa Majestee Britanique":"Get right inside a fearless envoy extraordinary at Her Majesty Queen of Britain' service",30,0x80D0FF);
            pstr("Republique Unifiee",_DY+10,0xFFD090);
            pstr(!lang?"Prenez l'air dans les rangos d'un courageux legionnaire du 1er regiment parachutistes d'infanterie de marine":"Take off in courageous legionnaire of the 1st RPIM's regulation shooes",_DY+30,0xFFD090);
            TextClipX1=_DX+10;
            TextClipX2=SX-10;
            pstr("Almouchie",10,0xFFFF70);
            pstr(!lang?"Defendez dans les airs l'honneur de votre mere patrie, la majestueuse Russie, victime de dangeureux terroristes":"Take in the air the defense of our mother country, the majestic Russia, victimized by dangerous terrorists",30,0xFFFF70);
            pstr(!lang?"Bas-Wanana":"Low-Wanana",_DY+10,0x70FF70);
            pstr(!lang?"Deguisez vous en digne guerrier fanatique pour tracer en lettres de feu dans le ciel les commandements de la morale de Dieu":"Turn into a fanatic warior and engrave commandments of God's true moral in the sky with glowing characters",_DY+30,0x70FF70);
            break;
        case 1 ... 4:
            TextClipX1=10;
            TextClipX2=SX-10;
            pstr(scenar[phaz-1][phaztxt][lang],_DY/3,colcamp[phaz-1]);
            break;
        case 5:
            TextClipX1=TextClipX2=0;
            pstr("FACHODA Complex v0.1",10,0xFFFFFF);
            pstr("code, gfx & sfx : RiXed",SY-20,0xFFFFFF);
            pstr(!lang?"Bonjour, Vous allez choisir en cliquant dessus le camp pour lequel vous desirez combattre. Ils sont tous les quatre tres recommandables. Puis vous lirez attentivement les instructions pour comprendre ce que l'on attend de vous. Mais pour l'instant, cliquez par ici s'il vous plait.":"Hello ? You are going to choose with your mouse the camp for wich you'd like to fight untill death. There are four of wich, all very recommendable. Then, you will mindfully read the instructions to understand what is expected from you. But for now, please click hereabout.",SY/3,0xFFFFFF);
            break;
        }
        plotcursor(xmouse,ymouse);
        buffer2video();
        xproceed();
        if (kreset(0)) {
            switch (phaz) {
            case 5:
                phaz=0;
                break;
            case 0:
                if (xmouse<_DX) i=0; else i=1;
                if (ymouse<_DY) i+=2;
                phaz=i+1;
                phaztxt=0;
                break;
            default:
                if (++phaztxt>3) {
                    TextClipX1=TextClipX2=0;
                    affpresent();
                    free(presentimg);
                    buffer2video();
                    return i;
                }
            }
        };
        if (kreset(gkeys[kc_esc].kc)) {
            if (phaz==5) return(-1);
            else if (phaz==0) phaz=5;
            else phaz=0;
        }
    } while (1);
}
*/
void redefinekeys() {
    int i,jdep,j;
    char msg[200];
    int nbl=SY/10;
//  darkpresent();
    for (j=0; j<NBKEYS; j++) {
        affpresent(0,0);
        jdep=j-(nbl-1);
        if (jdep<0) jdep=0;
        for (i=0; i<nbl && i<NBKEYS; i++) {
            if (i+jdep==j) {
                sprintf(msg,"Type new key for %s",gkeys[j].name);
                pstr(msg,i*10,0xFFFFFF);
            } else {
                sprintf(msg,"%s : %d",gkeys[i+jdep].name,gkeys[i+jdep].kc);
                pstr(msg,i*10,0xE0E0E0);
            }
        }
        buffer2video();
        gkeys[j].kc=getscancode();
    }

    keys_save();
}
