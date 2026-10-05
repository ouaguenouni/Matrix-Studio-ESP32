# Fiche atelier — cadeau « INGE / AFTER HOURS »

**Objectif :** boîtier complet pour un écran LED, à offrir le 9 octobre 2026.
Remplace le cadre photo. Source paramétrique OpenSCAD et STL de prototype fournis.
**Ne pas lancer la coque complète avant le contrôle des pièces réelles.**

## Aspect souhaité

Coque anthracite, finition régulière, vis arrière discrètes, petite platine de
connecteurs contrastée. Gravure arrière « INGE / AFTER HOURS ». Choisir un PETG
et un profil déjà maîtrisés par l'atelier ; l'aspect final dépend du filament,
du réglage et de l'orientation, pas seulement du modèle d'imprimante.

## Encombrement et pièces

Coque : 182,8 × 102,8 × 60 mm. Ensemble avec couvercle et platine : profondeur
maximale 66 mm. Toutes les pièces tiennent séparément sur un plateau 220 × 220 mm,
avec environ 5 mm de bordure autour de la coque.

- 1 coque `shell.stl`, face avant contre le plateau.
- 1 couvercle `lid.stl`, face plate contre le plateau.
- 1 platine `ports.stl`, gravures POWER / USB vers le haut.
- 1 support ESP32 `carrier.stl`, base contre le plateau.
- 4 clips `clip.stl` et 2 pieds `stand.stl`.
- 1 éprouvette `coupon.stl` à imprimer en premier.

STL en millimètres, géométries fermées vérifiées. Pas de G-code : trancher avec
le profil de l'imprimante réellement utilisée. Point de départ : buse 0,4 mm,
4 parois, remplissage 15–20 %, couches 0,2 mm pour les essais, éventuellement
0,16 mm pour les pièces visibles. Contrôler les ponts et surplombs des passages
pour colliers et des bossages arrière ; supports locaux si le trancheur l'exige.

## Vérification sur place avec le matériel

Le panneau est un Waveshare P2.5 64×32, nominalement 160 × 80 mm. Vérifier la
profondeur arrière, le dégagement des connecteurs branchés et la portée des clips
sur le cadre plastique, sans appui sur les soudures. Confirmer que la lèvre avant
ne masque aucune LED.

Carte identifiée par son propriétaire : **ESP32 NodeMCU WiFi Bluetooth Module
ESP32 WROOM 32 Development Board**, version USB-C photographiée. Ce nom ne
précise pas la révision mécanique. Mesurer le circuit ESP32 USB-C, ses quatre trous et son épaisseur avec les fils.
Le motif de perçage du support est **provisoire** (modifiable dans `enclosure.scad`).
Le support est démontable ; la position de l'antenne et les fils doivent rester
éloignés autant que possible du dos du panneau.

Sélectionner une courte rallonge **USB-C avec données** à fixation panneau et une
embase d'alimentation 5 V correspondant au bloc secteur. Vérifier les plans de ces
connecteurs avant de modifier la platine et l'éprouvette : le trou de 8 mm est un
alésage de fixation provisoire, pas une affirmation du diamètre du connecteur
actuel. Les deux ouvertures servent à POWER et USB / FLASH.

Tester une éprouvette, un support et un clip avant les grandes pièces. Ajuster les
jeux si nécessaire. Les avant-trous de 2,6 mm sont à valider puis tarauder pour M3 ;
choisir les longueurs de vis réelles sans atteindre le panneau ni les composants.
BOOT et EN restent accessibles couvercle retiré. Le prototype n'a pas de bouton
extérieur ; un poussoir nécessiterait la position mesurée du bouton.

## Montage et contrôles

Fixer le panneau, puis le support au couvercle. Attacher séparément les fils de
puissance et le ruban HUB75 aux guides périphériques. Garder une boucle de service
pour ouvrir le couvercle et fixer la retenue des câbles au boîtier. Ne pas obstruer
les évents. Ajouter des patins antidérapants aux deux pieds.

Le bloc secteur secteur reste **à l'extérieur**. Vérifier le circuit d'alimentation
avant de brancher simultanément l'entrée 5 V et l'USB d'un ordinateur : la carte
actuelle n'a pas de protection de sélection d'alimentation confirmée.

Avant emballage : contrôle USB/flash selon la procédure électrique validée,
connexion Wi-Fi, stabilité avec câbles branchés, températures en fonctionnement
prolongé, absence de fils pincés, et ouverture facile pour maintenance.

## Atelier conseillé

Artilect : https://www.artilect.fr/impression-3d — parc annoncé compatible avec
le gabarit, PETG et parcours d'initiation/réservation. Demander un accompagnement
pour la finition d'un cadeau et vérifier un créneau avant le 9 octobre. Aucune
réservation, prise de contact ou commande n'a été faite par l'assistant.

## Façade plexiglas fournie par le propriétaire

QWORK, lot de 10 plaques transparentes **130 × 180 mm**. Découper une fenêtre
**164 × 84 mm**, rayon des coins 1 mm, suivant `acrylic-front-164x84.svg` (échelle
1:1 en mm). Confirmer la matière sur l'emballage et choisir le procédé de coupe
adapté. L'épaisseur n'est pas précisée dans le titre produit : **la mesurer avant
l'impression de la coque** et régler `acrylic_thickness` (hypothèse actuelle : 2 mm).

Imprimer aussi `front_spacer.stl`, à plat. Montage : rebord avant, plexiglas,
entretoise périphérique, panneau LED, puis clips. Le jeu latéral est de 0,3 mm
par côté ; une fine garniture souple en périphérie absorbe le jeu en épaisseur.
Vérifier qu'aucune LED ne touche la plaque et qu'aucun clip ne la contraint trop.
La façade reste démontable par l'arrière, sans collage définitif.

Le canal inférieur guide les fils d'alimentation ; les attaches supérieures
maintiennent le ruban HUB75. Garder des courbes douces, une boucle pour ouvrir
le couvercle et les évents libres.
