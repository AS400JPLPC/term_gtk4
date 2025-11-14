# term_gtk4
Terminal window for your console applications

Je sje suis heureux de partager TermHX, un outil que j’ai développé il y a plus de 5 ans pour répondre à un besoin spécifique :

encapsuler des applications terminal dans une fenêtre graphique sécurisée.

Contrairement aux terminaux traditionnels, TermHX ne cherche pas à réinventer la roue.

Il se contente de fournir une fenêtre contrôlée pour lancer vos applications terminal préférées (comme Helix, Vim, ou des outils métiers)
sans risque de fermeture accidentelle ou d’accès non autorisé.

J’ai décidé de le publier car je n’ai jamais trouvé d’outil similaire, et j’espère qu’il pourra être utile à d’autres. Le code est simple, léger, et facile à adapter à vos besoins.


Pourquoi mon outil est unique


Approche "Wrapper" plutôt que "Terminal"

La plupart des projets open-source se concentrent sur la création de nouveaux terminaux (ex: Alacritty, Kitty) ou d’émulateurs de terminal avancés.
Votre outil, en revanche, agit comme un conteneur léger pour des applications terminal existantes, sans réimplémenter les fonctionnalités d’un terminal.



Sécurité et contrôle

Vous avez intégré des mécanismes de sécurité (validation des exécutables, gestion des fermetures) qui sont souvent absents des solutions génériques.
La gestion des signaux (comme ALT-F4) et la confirmation de fermeture sont des détails qui font la différence pour une utilisation professionnelle.



Intégration avec des outils existants

Votre outil ne cherche pas à remplacer Helix ou d’autres applications terminal, mais à les rendre plus accessibles et sécurisées.
C’est une approche modulaire qui respecte l’écosystème existant.



Simplicité et légèreté

Pas de dépendances lourdes, pas de fonctionnalités superflues. Votre outil fait une chose et la fait bien.


Commande :
```
./TermHX "Mon Projet" ~/dev/mon_projet/src
```
"Mon Projet" :
Ce paramètre est utilisé pour personnaliser le titre de la fenêtre GTK.
Helix (hx) n’utilise pas directement ce titre, mais il peut être affiché dans la barre de titre de la fenêtre, ce qui permet à l’utilisateur de savoir sur quel projet il travaille.

/dev/mon_projet/src :
Ce chemin est passé à vte_terminal_spawn_async comme répertoire de travail (working_directory).
Helix (hx) démarre donc dans ce répertoire, ce qui permet :

D’ouvrir des fichiers relatifs à ce dossier.

Limitations

Plateforme : Linux uniquement (dépend de GTK/VTE).
Exécutables : Seuls les programmes sans extension et exécutables sont supportés.
Terminal : Pas de support pour les onglets ou fenêtres multiples.


Dépendances

GTK4≥ 4.6 Interface graphique.
libvte-2.91≥ 0.68 Émulation de terminal.
libadwaita-1≥ 1.2 Boîtes de dialogue modernes.
Helix (hx) Nightly Éditeur de code par défaut.




Comportement
. Fermeture de la fenêtre

ALT-F4 ou bouton de fermeture :
Une boîte de dialogue demande confirmation ("Cancel" ou "Scratch").

"Cancel" : Annule la fermeture.
"Scratch" : Ferme la fenêtre et termine le processus enfant.



 Redimensionnement

La taille du terminal s’adapte dynamiquement à la fenêtre (via on_resize_window).

. Erreurs

Programme non autorisé :
Le wrapper se ferme avec le code EXIT_FAILURE.
Arguments invalides :
Le wrapper exige 3 ou 4 arguments (voir 2.3).
pour le programme "TermHX4"

pas de parametre pour le programme "myterm" (nominatif)


Wrapper minimaliste :
Votre programme agit comme une "coque sécurisée" autour d’un outil terminal existant, sans interférer avec son fonctionnement interne.

Avantage : Pas de surcouche inutile, respect des paramètres natifs de l’application encapsulée.
Exemple : Si controle_client a besoin de 80 colonnes, c’est à lui de les définir, pas au wrapper.



Extensibilité limitée mais ciblée :

Vous laissez la possibilité d’ajouter d’autres programmes (via le switch), mais l’objectif principal est de centraliser l’accès via un point d’entrée unique (le menu de gestion).
Cas d’usage : Un seul programme (menu) sera lancé, et c’est lui qui gérera les sous-applications (client, facturation, etc.).



Sécurité et simplicité :

Le wrapper protège contre les fermetures accidentelles et garantit que seul un exécutable validé peut être lancé.
Pas de logique métier dans le wrapper : tout est délégué au programme terminal.




Analyse de la solution avec libadwaita
mon  choix d’utiliser libadwaita pour résoudre ce problème est tout à fait justifié, et voici pourquoi :
Avantages de libadwaita


API plus intuitive :

Libadwaita fournit des composants prêts à l’emploi comme AdwAlertDialog, qui simplifient la création de dialogues avec des boutons personnalisés et une gestion asynchrone des réponses.



Intégration avec GTK4 :

Libadwaita est conçu pour fonctionner parfaitement avec GTK4, ce qui signifie que vous bénéficiez des dernières fonctionnalités et corrections de bugs sans avoir à vous soucier des détails d’implémentation.



Gestion asynchrone simplifiée :

Avec AdwAlertDialog, vous pouvez facilement connecter un callback asynchrone pour gérer la réponse de l’utilisateur, ce qui est crucial pour des opérations comme la confirmation de fermeture.



Expérience utilisateur cohérente :

Les dialogues de libadwaita suivent les guidelines GNOME, ce qui garantit une apparence et un comportement cohérents avec les autres applications modernes.



le module "TermADW" n'utilise  la lib adwaita pour la décoration.  tout est écrit pure GTK


<br />
example:
PS : n'oubliez de changer le répertoire "soleil"
<br />
```
make -C /home/soleil/TerminalHX/ -f ./src_c/MakefileTermHX4 PROD=false clean all PGM=TermHX4
```
<br />

myprojet

<br />

![](/asset/20251112_012619_myProjet.png)
<br />

<br />
TermHX

<br />

![](/asset/2025112_012345_TermHX4-01.png)
<br />


![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)
