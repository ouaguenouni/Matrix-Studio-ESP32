# Fiche atelier — cinq boîtiers INGE / AFTER HOURS

Dossier numérique complet, révision 3 du 5 octobre 2026. **Ajustement matériel
non vérifié.** Apporter le panneau, l'ESP32, les câbles branchés et l'acrylique.
Composants confirmés : panneau LED, ESP32, plaque acrylique et câblage uniquement.
L'alimentation secteur reste à l'extérieur.

## Choisir une seule variante

| Variante | Largeur × hauteur × profondeur, mm | Acrylique 1 mm | Bac à câbles |
| --- | --- | --- | --- |
| Line | 190 × 110 × 94,5 | Découper à 164 × 84 | 14 mm |
| Orbit | 202 × 122 × 96,5 | Découper à 164 × 84 | 20 mm |
| Gallery | 206 × 156 × 96,5 | Feuille entière 180 × 130 | 20 mm |
| Vault | 206 × 156 × 122,5 | Feuille entière 180 × 130 | 40 mm |
| Console | 202 × 122 × 108,5 | Découper à 164 × 84 | 30 mm |

Vault offre le plus de place pour les fils ; Line occupe le moins de place.
La profondeur inclut la plaque arrière, mais pas les têtes de vis ni les lèvres
des pieds. Les pieds ajoutent 4 mm sous le boîtier. Ouvrir `viewer.html` pour
comparer et voir le démontage ; `comparison.png` fonctionne sans WebGL.

## Avant d'imprimer le grand boîtier

- Vérifier le panneau : 160 × 80 × 14,5 mm nominaux ; quatre inserts M3 arrière
  extérieurs espacés de 125 × 65 mm. Mesurer la profondeur utile des inserts pour
  choisir les vis sans toucher l'électronique. Le dessin fourni est une référence.
- Mesurer le PCB ESP32 et ses trous. Valeurs provisoires : 58 × 29 mm, trous
  52 × 23 mm. Support réglable : entraxe longitudinal 30–64 mm, transversal
  0–32 mm. Ne pas agrandir les trous du PCB pour utiliser des vis M3.
- Tester `coupon.stl`, `carrier.stl`, un `rail.stl`, un `spacer.stl` et `ports.stl`.
  Les rainures acryliques sont ouvertes sur le dessus : 1,0 / 1,2 / 1,4 mm.
  Les petits trous, de gauche à droite : 1,6 / 1,8 / 2,0 / 2,2 / 2,4 / 2,6 /
  2,8 / 3,0 / 3,4 mm. Les grands trous : 12,2 / 12,4 / 12,6 mm.
- Vérifier les passages de câbles : 22 × 14 mm pour l'alimentation, 32 × 18 mm
  pour l'USB. Les connecteurs existants passent dans les ouvertures ; aucune
  rallonge USB supplémentaire n'est imposée. Retenir les câbles par des sangles.
- Contrôler les câbles branchés, les courbures et l'accès BOOT/EN avec couvercle
  retiré. Réserve modélisée : 20 mm derrière le panneau, entre les traverses.

## Quantités par boîtier

Une pièce de chaque : `bezel`, `keeper`, `body`, `lid`, `carrier`, `cassette`,
`ports`. Deux `bridge`, deux `rail`, quatre `spacer`, deux `feet`.
Soit **17 pièces installées**, plus un `coupon` d'essai.
`ports_usb_c` remplace `ports` seulement après sélection et mesure d'un adaptateur
USB de panneau rond ; ne pas imprimer les deux pour le montage standard.

Pour un PCB acceptant des M3 : 30 vis/boulons M3, 14 écrous M3 et 4 vis M2.
Choisir les longueurs en fonction des empilements réels. Le support PCB traverse
le PCB + une entretoise de 20 mm + un rail de 3 mm + rondelle/écrou. Les vis du
panneau traversent une traverse de 3 mm puis ses inserts. Ne jamais atteindre les
composants. Tester/préparer les avant-trous imprimés de 2,6 mm (M3) et 1,6 mm (M2).
Les logements d'écrous du support mesurent 6,1 × 2,5 mm : vérifier les écrous.
Prévoir rondelles, sangles souples, patins et adhésif démontable pour les pieds.

## Impression et montage

STL en millimètres, à importer sans changement d'échelle. Le SVG de découpe
ajoute 1 mm de marge autour du tracé : page 166 × 86 ou 182 × 132 mm. Importer
à 100 % et découper le tracé ; ne pas redimensionner la page à la taille de plaque.
Une pièce par fichier ;
dupliquer les pièces selon les quantités. Tous les fichiers sont posés à Z=0.
L'encombrement maximal est 206 × 156 mm et tient sur un plateau supposé
220 × 220 mm, marge de 5 mm pour bordure comprise. Employer le profil validé de
l'atelier ; inspecter les ponts, évents et logements d'écrous dans le trancheur.
Aucun G-code machine n'est fourni et aucun essai d'impression n'est revendiqué.

1. Mettre l'acrylique dans la façade démontable, puis visser la bague `keeper`.
2. Visser façade et corps ; fixer le panneau par ses inserts arrière aux deux
   traverses, puis les traverses aux sièges du corps. Aucun appui sur les LED.
3. Sur l'intérieur du couvercle : fixer support et bac ; ajuster les rails et
   monter le PCB sur quatre entretoises. Composants orientés vers le couvercle,
   connexions sous le PCB vers le panneau. Consulter la vue éclatée.
4. Fixer la plaque de câbles à l'extérieur du couvercle. Attacher les fils,
   conserver une boucle pour ouvrir et éloigner les faisceaux de l'antenne.
   Boucler doucement le surplus dans le bac ; ne pas serrer la nappe HUB75.
5. Fermer avec quatre vis ; poser les deux pieds avec patins et adhésif démontable.
6. Tester stabilité avec câbles, retenue, affichage, USB, Wi-Fi et température
   en usage prolongé. Garder l'entrée externe 5 V de l'ESP32 débranchée lorsqu'il
   est alimenté par l'USB de l'ordinateur, conformément au projet.

Le guide anglais `PRINT-GUIDE.md` détaille les quantités par assemblage. Après
une modification des paramètres, régénérer les STL et `validation.json` avant
l'impression. L'ancien dossier `inge-after-hours` n'est pas cette famille de cinq.
