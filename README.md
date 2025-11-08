# EyeToy-et-MPEG
Quelques scripts pour extraire les images en MPEG avec une caméra EyeToy. C'est vraiment du code brut, juste là pour montrer que ça fonctionne.

Le fichier `Dump.c` est juste une version adaptée de ce bout de code : https://pastebin.com/461GCy2D 
Il a été écrit par [Florin9doi](https://github.com/Florin9doi)
Il y a juste quelques modifications pour bien enregistrer l'image complète. 

Le programme se compile avec la commande `gcc Dump.c -o Dump -lusb-1.0` et il faut avoir installé `libusb`. Ça fonctionne sous macOS (testé sur Intel) et Linux.
Il faut le lancer après branché la webcam EyeToy, il va simplement enregistrer chaque *frame* dans un sous-dossier `dumps` (à créer) et stocker quelques secondes d'images.

Si vous êtes sous Linux, il faut désactiver le pilote des webcams OV519, c'est dans les commentaires au début. Ce n'est pas nécessaire sous macOS.

Le script `IPU.sh` efface juste les 16 premiers octets des fichiers et ajoute l'extensions `.ipu`. Il faut le rendre exécutable et il s'occupe des fichiers dans le dossier courant.

Pour décoder les images `.ipu`, vous aurez besoin de https://github.com/samehb/IPUDecoder et https://github.com/samehb/IPUDecoderGUI (sous Windows).
