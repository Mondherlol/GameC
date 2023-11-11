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
C'est pour éviter que le contenu des fichier .h ne soient ajoutés si ils y sont déjà

## Render
Le processus de générer l'image finale à afficher.

