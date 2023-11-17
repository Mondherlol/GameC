# Principes fondamentaux

## Vertex (ou Sommet)
Un point dans l'espace en 3D défini par des coordonnées (x, y, z). C'est similaire à un vecteur, mais sans concept de direction.

## Les Shaders
Des programmes qui effectuent des opérations de rendu graphique.

Dans OpenGL, on a :

- **Vertex Shader :** Relie les sommets pour former un objet 2D/3D.
- **Fragment Shader :** Calcule la couleur finale de chaque pixel après que le Vertex Shader a été appliqué.

En gros, le Vertex Shader s'occupe du dessin, et le Fragment Shader s'occupe de la coloration.

## Quad (ou Quadrilatère)
Un quadrilatère à afficher à l'écran, généralement utilisé comme rectangle.

## Buffer
C'est une zone de mémoire TEMPORAIREMENT utilisée pour stocker une donnée quelconque 

## VAO (Vertex Array Object)
Un objet open gl sous forme de tableau qui va contenir les vertex (sommets) du quad

## VBO (Vertex Buffer Object)
Un objet OpenGL qui va stocker les données des sommets, comme les positions et les couleurs

## EBO (Element Buffer Object)
Un objet OpenGL qui va stocker l'INDICE de L'ORDRE des sommets. => Logique, un rectangle et un carré vont pas utiliser le même nombre de sommets par exemple.

## Texture
Une image qu'on va venir coller sur une surface 2d ou 3d, pour donner une "texture" visuelle aux objets

## Pragma
Un préprocesseur, C'est pour éviter que le contenu des fichier .h ne soient ajoutés si ils y sont déjà

## Render
Le processus de générer l'image finale à afficher.

## Update / Game Loop
Pour que le jeu s'execute de la même façon et A la MEME vitesse sur toutes les machines, au lieu d'utiliser simplement une boucle while pour le rendu de notre jeu et nos actions. On va alors enregistrer des variables tel que le temps de la derniere image rendue, le temps entre deux images, etc...
Comme ça si jamais le jeu tourne sur un pc deux fois plus rapide, il ne s'executera pas deux fois plus rapidement.
Regarder cette vidéo pour mieux comprendre le principe d'une fonction update : https://www.youtube.com/watch?v=lW6ZtvQVzyg&t=431s&ab_channel=VittorioRomeo

## AABB (Axis-Aligned Bounding Box)
C'est un rectangle aligné avec les axes de l'écran, C'EST A DIRE que les segments hauts et bat du rectangle sont parallèles avec la bordure de la fenêtre de jeu, et les côtés aussi. 
Il est définit par sa largeur et sa hauteur , comme tout rectangle MAIS aussi par sa position X et Y. On compte la position X et Y à partir du point supérieur gauche.
Pour une image plus détaillée et comprendre l'intérêt voir ici : http://sdz.tdct.org/sdz/eorie-des-collisions.html


## Comprendre les collisions entre AABB :
https://noonat.github.io/intersect/