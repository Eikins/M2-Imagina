# HMIN 317 - Moteurs de Jeux : Prise en main de Qt Creator, Git et OpenGL ES 3.0

## Transformations

Un `GameObject` doit pourvoir subir différentes opérations mathématiques pour pouvoir le positionner et orienter dans l'espace.

Pour se faire, on utilise les **matrices de transformations**.

Les trois transformations nécessaires pour représenter un objet sont :
* Les translations $T$
* Les rotations $R$
* Les homothéties $S$

La multiplication de matrices est l'équvalent de la composition des transformations, nous pouvons donc définir une matrice représentant la transformation de notre objet (Model Matrix $M$).

$$M = T \times R \times S$$

Pour simplifier les usages, nous ne travaillons pas directement avec des matrices, mais avec des objets plus simples représentant ces transformations qui seront convertis en matrices.

Sous Qt, nous avons les éléments suivants :

* Les matrices sont représentées par des `QMatrix4x4`
* Les translations et homothéties sont représentée par des `QVector3D`
* Les rotations sont représentées par des `QQuaternion`

On peut donc par exemple, appliquer une transformation à notre objet
```C++
QMatrix4x4 transform;
QVector3 translation = QVector3(0.0, 0.0, -5.0);
// Rotation of 45° around Y
QQuaternion rotation = QQuaternion::fromAxisAndAngle(QVector3(0, 1, 0), 45.0);
QVector3 scale = QVector3(2.0, 1.0, 2.0);

transform.translate(translation);
transform.rotate(rotation);
transform.scale(scale);
```

*Pour la 2D, il existe l'équivalent [`QTransform` 🔗](https://doc.qt.io/qt-5/qtransform.html#details)*

## Dessin en 3D


## Génération de terrain avec une Heightmap

### Génération d'un plan subdivisé

La première étape consiste à générer un plan subdivisé sur lequel nous allons ensuite modifier la hauteur des sommets.

Pour se faire, nous utilisons un [Triangle Strip 🔗](https://en.wikipedia.org/wiki/Triangle_strip).

*Une autre méthode est d'utiliser des éléments, mais moins optimal.   [`Mesh.Plane` 🔗](https://github.com/Eikins/3D-Sea-Project/blob/master/sea3d/core/mesh.py#L118)*
```C++
struct VertexData
{
    QVector3D position;
    QVector2D texCoord;
};


void Terrain::generatePlaneGeometry(int width, int length, int widthSegments, int lengthSegments)
{
    int xCount = widthSegments + 1;
    int zCount = widthSegments + 1;

    int vertexCount = xCount * zCount;
    int indexCount = vertexCount + length;

    std::vector<VertexData> vertices(vertexCount);
    std::vector<GLushort> indices(indexCount);

    // Create vertices by lines
    // 0 -- 1 -- ... -- W
    // |    |     |     |
    // W+1  . --  . --  2W
    // |    |     |     |
    // . -- . --  . --  .
    // |    |     |     |
    // . -- . --  . --  LW

    for (int x = 0; x < xCount; x++)
    {
        for (int z = 0; z < zCount; z++)
        {
            float u = ((float) x) / xCount;
            float v = ((float) z) / zCount;
            QVector3 position(u * width, v * length);
            QVector2 texCoord(u, v);
            vertices.push_back({position, texCoord});
        }
    }

    // TODO: Indices
    // 0 w+1 1 w+2 3 .... w-1 2w-1 2w-1
    // ...
    // pair => left to right
    // impair => right to left

    // Transfer vertex data to VBO 0
    glBindBuffer(GL_ARRAY_BUFFER, vboIds[0]);
    glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(VertexData), &vertices[0], GL_STATIC_DRAW);

    // Transfer index data to VBO 1
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIds[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(GLushort), &indices[0], GL_STATIC_DRAW);
}
```

