# DIY Synth 01 — Manuel d'utilisation

Un petit synthé de voyage, fait maison, à brancher sur un Pocket Operator (PO-33), une carte son ou des enceintes amplifiées.
On joue les notes avec deux **ribbons** (des bandes tactiles) et on sculpte le son avec 3 potards, 3 boutons et un encodeur.

---

## 1. Vue d'ensemble

```
   ┌──────────────────────────────────────────────┐
   │  ÉCRAN                                        │
   │                                               │
   │  [POT 1]  [POT 2]  [POT 3]      (ENCODEUR)    │
   │                                               │
   │  (BTN 1)  (BTN 2)  (BTN S)   ← avec LED       │
   │                                               │
   │  ══════ RIBBON 1 : touches blanches ══════    │
   │  ══════ RIBBON 2 : touches noires   ══════    │
   └──────────────────────────────────────────────┘
```

| Élément | Rôle principal |
|---|---|
| **Ribbon 1** | Les 7 notes blanches : C D E F G A B (do ré mi fa sol la si) |
| **Ribbon 2** | Les 5 notes noires : C# D# F# G# A# |
| **Potard 1** | Filtre (CUTOFF) |
| **Potard 2** | Résonance (RESO) |
| **Potard 3** | Queue des notes (RELEASE) |
| **Boutons 1, 2, S** | Allumer / éteindre chaque oscillateur |
| **Encodeur** (bouton qui tourne et qui clique) | Octave, menu, raccourcis |

---

## 2. Jouer

- **Pose un doigt sur un ribbon** : une note joue. Lève le doigt : elle s'arrête.
- Chaque ribbon est **découpé en zones**, une par note : bas du ribbon = note grave, haut = note aiguë.
- Les deux ribbons ensemble font **une octave complète de 12 notes**, comme un clavier de piano.
- Le synthé joue **une seule note à la fois**. Si tu touches les deux ribbons, c'est le **dernier touché** qui joue.
- **Tourner l'encodeur** change l'**octave** (de -2 à +2) pour jouer plus grave ou plus aigu.

### Tenir une note : le HOLD
**Double-clic sur l'encodeur** : la note jouée reste tenue même si tu lèves le doigt.
Pratique pour avoir les deux mains libres et triturer les potards.
- Une **nouvelle note** sur un ribbon annule le HOLD.
- Un **nouveau double-clic** l'arrête aussi.

---

## 3. Le son, expliqué simplement

Le chemin du son :

```
 3 oscillateurs  →  filtre  →  enveloppe  →  volume  →  sortie
  (la source)      (la couleur) (la forme dans le temps)
```

### Les oscillateurs : la source du son
Il y en a trois, qui jouent tous la même note :

| Oscillateur | Ce qu'il fait |
|---|---|
| **1** | La note jouée |
| **2** | La même note, très légèrement désaccordée : ça rend le son plus **épais** (effet « chorus ») |
| **S** (sub) | La note **une octave plus bas** : ça ajoute du **corps** et des basses |

Chacun peut avoir sa propre **forme d'onde**, c'est-à-dire son timbre de base :

| Forme d'onde | Son |
|---|---|
| **SAW** (dent de scie) | Brillant, riche, un peu agressif. Le son « synthé » classique. |
| **SQR** (carré) | Creux, un peu nasillard, façon jeu vidéo rétro. |
| **TRI** (triangle) | Doux, proche d'une flûte. |
| **SINE** (sinus) | Pur et rond, sans aucune aspérité. |

### Le filtre : la couleur
- **CUTOFF** (potard 1) : à gauche le son est **sourd et étouffé**, à droite il est **brillant et ouvert**. C'est le réglage le plus expressif : tourne-le en jouant.
- **RESO** (potard 2) : accentue la zone du CUTOFF. Monté, le son **siffle** et devient plus « acide ». Combine-le avec des mouvements de CUTOFF.

### L'enveloppe (ADSR) : la forme de la note dans le temps
Elle décide comment le volume d'une note **démarre, se tient et s'arrête** :

```
 volume
   │      /\
   │     /  \_______________
   │    /                   \
   │   /                     \
   └──A───D────── S ─────────R──→ temps
      doigt posé           doigt levé
```

| Réglage | Question à laquelle il répond | Où |
|---|---|---|
| **A**ttack | Combien de temps pour que le son **monte** ? | Clic maintenu + potard 1 |
| **D**ecay | Combien de temps pour **redescendre** vers le niveau tenu ? | Clic maintenu + potard 2 |
| **S**ustain | À quel **niveau** le son se tient tant que le doigt est posé ? | Clic maintenu + potard 3 |
| **R**elease | Combien de temps le son **traîne** après avoir levé le doigt ? | Potard 3 |

Le DECAY ne s'entend que si le SUSTAIN est sous 100 %.

---

## 4. Toutes les commandes

### Les gestes de base
| Geste | Effet |
|---|---|
| Tourner **potard 1** | CUTOFF : sourd ↔ brillant |
| Tourner **potard 2** | RESO : résonance du filtre |
| Tourner **potard 3** | RELEASE : queue des notes |
| Tourner **l'encodeur** | OCTAVE |
| **Appui court** sur bouton 1, 2 ou S | Allume / éteint cet oscillateur (sa LED suit) |

### Les raccourcis (gestes combinés)
| Geste | Effet |
|---|---|
| **Maintenir** un bouton d'oscillateur **+ tourner l'encodeur** | Change la **forme d'onde** de cet oscillateur |
| **Maintenir** un bouton d'oscillateur **+ cliquer l'encodeur** | **Verrouille** l'encodeur sur la forme d'onde de cet oscillateur : ensuite, tourner l'encodeur seul la change (une main reste sur le ribbon). La LED de l'oscillateur **clignote**. Un **clic simple** déverrouille. |
| **Maintenir le clic** de l'encodeur **+ tourner l'encodeur** | **DETUNE** : écart entre l'osc 1 et l'osc 2 (épaisseur du son) |
| **Maintenir le clic** de l'encodeur **+ tourner un potard** | Règle l'enveloppe : potard 1 = **ATTACK**, 2 = **DECAY**, 3 = **SUSTAIN** |
| **Maintenir** un bouton d'oscillateur **+ tourner n'importe quel potard** | **Volume de cet oscillateur**, de 0 à 200 % |
| **Double-clic** sur l'encodeur | **HOLD** : la note reste tenue |
| **Clic simple** sur l'encodeur | Ouvre le **menu réglages** |

Quand un potard est tourné dans une couche (normale ou clic maintenu), la valeur **saute** à la position du potard.

### Le menu réglages
Un **clic simple** ouvre le menu. **Tourner** change la valeur, **cliquer** passe au réglage suivant. Après le dernier, le menu se ferme. Il se ferme aussi tout seul après 6 secondes sans y toucher.

| # | Réglage | Utilité | Plage |
|---|---|---|---|
| 1 | **VOLUME** | Volume général, pour s'accorder au niveau du PO-33 | 0 à 100 % |
| 2 | **LEVEL 1** | Volume de l'oscillateur 1 | 0 à 200 % (départ 100 %) |
| 3 | **LEVEL 2** | Volume de l'oscillateur 2 | 0 à 200 % (départ 100 %) |
| 4 | **LEVEL S** | Volume du sub (les basses) | 0 à 200 % (départ 50 %) |
| 5 | **GLIDE** | La note **glisse** vers la suivante au lieu de sauter, quand on enchaîne sans lever le doigt | OFF à 1000 ms |
| 6 | **TRANSPOSE** | Décale tout le clavier, pour jouer dans la tonalité d'un morceau | -12 à +12 demi-tons |
| 7 | **PITCH ENV** | Chaque note **démarre plus aiguë et plonge** vers sa vraie hauteur. C'est ce qui fait le « boum » d'un kick, ou un « piou » laser | OFF à +48 demi-tons |
| 8 | **PITCH TIME** | Vitesse de cette plongée : court = claquant, long = « piou » | 5 à 500 ms |

---

## 5. L'écran

### Vue d'ensemble
```
 1:SAW      2:SAW      S:SQR        ← forme d'onde de chaque osc ("---" = éteint)
 C#3        OCT +0  HOLD            ← note jouée, octave, HOLD si actif
            138.6 Hz                ← fréquence de la note
 CUT 2500   RES 1.2                 ← filtre
 A5 D200 S100 R150                  ← enveloppe (ms, et % pour S)
 [██████████░░░░░░░░░░░░░]          ← position du doigt sur le ribbon
```
L'oscillateur verrouillé sur l'encodeur est affiché **en inversé** (noir sur blanc).

### Popups
Dès que tu touches une commande, son **nom et sa valeur** s'affichent en gros pendant 1 seconde, par exemple `CUTOFF 2500 Hz`.

---

## 6. Recettes de sons

| Son | Formes d'onde (1 / 2 / S) | CUTOFF | RESO | A | D | S | R |
|---|---|---|---|---|---|---|---|
| **Lead classique** | SAW / SAW / SQR | moyen | bas | min | — | 100 % | court |
| **Pluck** (corde pincée) | SAW / SQR / off | moyen | moyen | min | ~200 ms | 0 % | court |
| **Basse ronde** | SQR / off / SINE | bas | bas | min | ~100 ms | ~30 % | court |
| **Nappe douce** (pad) | TRI / TRI / SQR | moyen | bas | ~1 s | — | 100 % | ~2 s |
| **Acide** | SAW / off / off | bas → monte en jouant | haut | min | ~150 ms | ~20 % | court |
| **Flûte** | SINE / TRI / off | haut | bas | ~80 ms | — | 100 % | moyen |
| **Kick** | SINE / off / SINE, LEVEL S à 200 %, OCTAVE -2 | très bas | bas | min | ~150 ms | 0 % | min |

Pour le **kick**, règle aussi dans le menu **PITCH ENV** vers +24 à +36 et **PITCH TIME** vers 20 à 40 ms, puis tape brièvement le bas du ribbon 1. Avec un PITCH TIME plus long (~150 ms), tu obtiens un son de laser façon jeu vidéo.

Astuce : active le **GLIDE** (menu, réglage 5) vers 100-200 ms et glisse le doigt sur le ribbon sans le lever.

---

## 7. Branchements

### Sortie audio
La sortie du DAC (PCM5102) est une **sortie ligne** :
- ✅ entrée du PO-33, carte son (entrée ligne ou guitare, gain bas au départ), enceintes amplifiées ;
- ❌ **pas de casque ni d'enceintes passives en direct** : il faut un ampli entre les deux.

### Câblage interne (Teensy 4.1)
| Composant | Broche | Teensy |
|---|---|---|
| Encodeur | CLK / DT / SW | pins 4 / 3 / 2 (alim 3.3V) |
| Boutons osc 1 / 2 / S | une patte sur la pin, l'autre en diagonale sur GND | pins 5 / 6 / 9 |
| LED sequins osc 1 / 2 / S | + sur la pin, − sur GND (résistance intégrée) | pins 10 / 11 / 12 |
| Ribbon 1 / 2 | sortie | pins 14 (A0) / 15 (A1), alim 3.3V |
| Potards CUTOFF / RESO / RELEASE | patte du milieu ; extrémités sur 3.3V et GND | pins 16 (A2) / 17 (A3) / 22 (A8) |
| Écran OLED | SDA / SCL | pins 18 / 19, **VCC sur VIN (5V)** |
| DAC PCM5102 | DIN / LCK / BCK | pins 7 / 20 / 21, VIN 3.3V, SCK sur GND |

⚠️ **Tout est en 3.3V sauf l'écran** (qui a son propre régulateur). Ne jamais mettre du 5V sur un composant qui renvoie un signal vers la Teensy.

---

## 8. Dépannage

| Symptôme | À vérifier |
|---|---|
| Une note joue toute seule | Le ribbon est-il pressé ou plié quelque part ? Regarder `RIB1=` / `RIB2=` dans le moniteur série : au repos, ils doivent être sous ~15. |
| L'écran s'éteint ou perd son image | Alimentation de l'écran (sur VIN 5V, fil bien enfoncé). |
| Un potard ne réagit pas ou saute | Mauvais contact sur la breadboard : souder des fils sur les pattes. Potard défectueux : le remplacer. |
| Un bouton ou une LED ne réagit pas | Soudure de la broche sur la Teensy. Bouton branché en **diagonale**. |
| Sifflement ou grésillement | Masses (GND) : chaque module sensible avec son propre fil GND vers la Teensy. |
| Rien ne sort | Le jack est-il sur une **entrée ligne** ? Le VOLUME du menu est-il à 0 ? Les 3 oscillateurs sont-ils éteints ? |

Le **moniteur série** (96000 bauds) affiche en continu l'état du synthé : valeurs des ribbons et des potards, note, octave, formes d'onde, oscillateurs allumés. C'est le premier réflexe en cas de doute.
