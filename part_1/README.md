# Partie 1 — Implémentation et comparaison de structures de données

Trois structures stockant une collection d'entiers : **tableau dynamique**,
**liste chaînée simple**, **table de hachage** (chaînage séparé, `TABLE_SIZE = 10007`).

## Fichiers

- `main.c` — les trois structures (tableau dynamique, liste chaînée, table de
  hachage) + le benchmark + `main()`, tout en un seul fichier
- `Makefile` — `make`, `make run`, `make clean`

## Complexité théorique

| Opération              | Dynamic Array          | Linked List (sans tail) | Hash Table (chaînage)     |
|-------------------------|------------------------|--------------------------|----------------------------|
| `insert_front`          | O(n) — décale tout      | O(1)                     | —                           |
| `insert_back`           | O(1) amorti     ""        | O(n) — pas de tail, parcourt tout | —                  |
| `hash_insert`           | —                       | —                         | O(1) moyen / O(n) pire cas |
| `get(index)`            | O(1)                    | O(n)                     | —                           |
| `find(value)`           | O(n)                    | O(n)                     | —                           |
| `hash_contains`         | —                       | —                         | O(1) moyen / O(n) pire cas |
| `remove_front`          | O(n) — décale tout      | O(1)                     | —                           |
| `free_collection`       | O(1) (un seul bloc)     | O(n) (n `free`)          | O(TABLE_SIZE + n)          |
| Mémoire / élément       | contigu, faible overhead| +1 pointeur/nœud, allocations éparses | +1 pointeur/entrée, buckets fixes |

Pire cas hash table : toutes les clés tombent dans le même bucket (mauvaise
fonction de hachage ou attaque par collision) → la chaîne dégénère en liste,
`hash_insert`/`hash_contains` passent en O(n).

## Résultats mesurés

Machine : macOS, build `-O2`, `clock_gettime(CLOCK_MONOTONIC)`, seed fixe (42),
2000 lookups aléatoires par mesure de `find`/`contains`. `remove_front` mesuré
en moyenne sur 20000 retraits (ou N si N < 20000).

```
Structure      N          insert (total s) find (avg s/op)  get[N/2] (s)     remove_front (avg s)
----------------------------------------------------------------------------------------------
DynamicArray   1000       0.000022         0.000000552      0.000000000      0.000000500
LinkedList     1000       0.000030         0.000000770      0.000000000      0.000000019
HashTable      1000       0.000022         0.000000007      N/A              N/A

DynamicArray   10000      0.000067         0.000003112      0.000000000      0.000002870
LinkedList     10000      0.000236         0.000008554      0.000003000      0.000000016
HashTable      10000      0.000106         0.000000006      N/A              N/A

DynamicArray   100000     0.000205         0.000016599      0.000000000      0.000024631
LinkedList     100000     0.000669         0.000055325      0.000050000      0.000000007
HashTable      100000     0.000584         0.000000030      N/A              N/A

DynamicArray   1000000    0.001437         0.000134521      0.000000000      0.000278360
LinkedList     1000000    0.007084         0.000581575      0.000399000      0.000000008
HashTable      1000000    0.006703         0.000002639      N/A              N/A
```

(Le tableau `insert` compare `insert_back` pour DynamicArray, `insert_front`
pour LinkedList — c'est l'insertion O(1) de chaque structure — et
`hash_insert` pour HashTable ; comparer `insert_back` DA vs `insert_front` LL
serait injuste, chacune est mesurée sur son opération rapide native.)

### Lecture des chiffres

- **`find` croît linéairement avec N** pour DynamicArray et LinkedList
  (×~8 quand N ×10), confirme le O(n). LinkedList est ~3-4× plus lent que
  DynamicArray à N égal : pointer chasing casse la localité cache que le
  tableau contigu exploite.
- **`get(index)` reste ~0 pour DynamicArray** (O(1), accès direct) alors qu'il
  croît linéairement pour LinkedList (O(n), doit parcourir depuis `head`).
- **`remove_front` reste quasi constant pour LinkedList** (8-19 ns, O(1) :
  juste détacher `head`) alors qu'il **croît avec N pour DynamicArray**
  (0.5 µs → 278 µs, O(n) : décalage de tous les éléments restants).
- **`hash_contains` est quasi-constant jusqu'à N=10000** (proche du
  `TABLE_SIZE`=10007, donc load factor ≤1, chaînes courtes), puis **remonte**
  à N=100000 et N=1000000 car le load factor dépasse 1 (`n/TABLE_SIZE` ≈ 10 et
  ≈100) : les chaînes s'allongent et `hash_contains` glisse vers O(n/TABLE_SIZE).
  Ça illustre concrètement pourquoi une table de hachage a besoin d'un
  redimensionnement (rehash) en production — ce TP la garde à taille fixe pour
  isoler l'effet du chaînage.

## Justification du choix selon les contraintes

- **Accès fréquent par index, peu d'insertions/suppressions en tête,
  taille connue à l'avance** → **Dynamic Array**. `get(index)` O(1) et
  localité mémoire imbattable pour un parcours (`find` le plus rapide des
  trois structures liées à un ordre). Coût : `insert_front`/`remove_front`
  O(n), à éviter si ces opérations sont fréquentes.

- **Insertions/suppressions fréquentes en tête, taille imprévisible,
  pas besoin d'accès par index** → **Linked List**. `insert_front` et
  `remove_front` O(1) garanti (pas de réallocation, pas de shift), au prix
  d'un `find`/`get` O(n) plus lent que le tableau (moins bonne localité) et
  d'un overhead mémoire par nœud (pointeur + allocation séparée par élément,
  fragmentation possible).

- **Test d'appartenance (`contains`) ou dédoublonnage sur gros volume, ordre
  sans importance** → **Hash Table**. O(1) moyen pour insert/contains, imbat
  les deux autres structures dès que N dépasse quelques centaines d'éléments
  (0.03 µs vs 16.6 µs pour `find` du DynamicArray à N=100000, soit ~500×
  plus rapide). Contrainte : dimensionner `TABLE_SIZE` (ou implémenter un
  rehash) pour garder le load factor bas, sinon dégradation vers O(n) comme
  observé au-delà de N=10000 dans les mesures ci-dessus ; pas d'ordre
  d'itération, pas d'accès par position.

En résumé : DynamicArray pour l'accès indexé, LinkedList pour les
mutations en tête à coût garanti, HashTable pour la recherche
d'appartenance sur gros volume.
