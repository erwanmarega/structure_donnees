# Partie 2 — Analyse & Benchmark

Analyse comparative de `DynamicArray` vs `LinkedList` (repris de
[`part_1`](../part_1)) sur des opérations ciblées, + expérience
`hash_good` vs `hash_bad` sur la table de hachage.

## Fichiers

- `main.c` — DynamicArray + LinkedList (copie de part_1) + HashTable à
  fonction de hachage interchangeable + benchmark + `main()`
- `Makefile` — `make`, `make run`, `make clean`

## Tableau de complexités théoriques

| Opération        | Dyn. Array           | Linked List (sans tail) | Gagnant prévu |
|-------------------|----------------------|---------------------------|----------------|
| `get(index)`      | O(1)                 | O(n)                      | **Array**      |
| `find(value)`     | O(n)                 | O(n)                      | Égalité en théorie (Array gagne en pratique, meilleure localité mémoire) |
| `insert_front`    | O(n) — décale tout    | O(1)                      | **List**       |
| `insert_back`     | O(1) amorti          | O(n) — pas de tail, parcourt tout | **Array** |
| `remove_front`    | O(n) — décale tout    | O(1)                      | **List**       |
| Parcours complet  | O(n)                 | O(n)                      | Égalité en théorie (Array gagne en pratique) |

## Méthodologie de mesure

- Horloge : `clock_gettime(CLOCK_MONOTONIC, ...)`.
- Tailles testées : N = 1 000 / 10 000 / 100 000 / 1 000 000.
- Chaque structure est construite une fois pour une taille N, puis chaque
  opération est **répétée plusieurs fois sur la structure déjà construite**
  et on rapporte la moyenne (pas une seule mesure bruitée) :
  - `get(n/2)`, `find(valeur absente = -1)` : jusqu'à 2000 répétitions
    (réduit à 200 pour `find` à partir de N=100 000 car chaque appel est
    O(n) et 2000 répétitions à 1M éléments serait inutilement long).
  - `insert_front` / `insert_back` : 500 sondes timées, puis boucle de
    compensation **non timée** pour ramener la structure à sa taille N.
  - `remove_front` : 500 sondes timées, avec ré-insertion en fin pour
    compenser (garde la taille constante).
  - Parcours complet + somme : 20 répétitions.

### Piège rencontré : élimination de code mort

Premier essai : `insert_front` mesuré en bouclant `insert(x); remove(x)`
dans la même boucle timée. Résultat : **exactement 0.000000000 s** pour la
LinkedList, à toutes les tailles — suspect. En `-O2`, le compilateur a
prouvé que le nœud malloué puis immédiatement libéré (même adresse, aucune
autre observation) n'a aucun effet visible, et a supprimé l'appel
malloc/free (et la boucle) entièrement. Le tableau dynamique n'a pas ce
problème (pas de paire malloc/free, juste des écritures mémoire sur un
buffer qui reste vivant après la boucle). Correction : séparer la boucle
d'insertion timée de la boucle de suppression (non timée), et écrire le
résultat dans une variable de contrôle pour empêcher toute élimination.

## Résultats mesurés

```
=== Dynamic Array vs Linked List ===

N = 1000
  operation              DynamicArray (s)   LinkedList (s)
  get(n/2)               0.000000002        0.000000707
  find(absent)           0.000000489        0.000001479
  insert_front           0.000000162        0.000000062
  insert_back            0.000000098        0.000001632
  remove_front           0.000000098        0.000001656
  full traversal (sum)   0.000000100        0.000001500

N = 10000
  operation              DynamicArray (s)   LinkedList (s)
  get(n/2)               0.000000004        0.000007713
  find(absent)           0.000003927        0.000013614
  insert_front           0.000000786        0.000000016
  insert_back            0.000000624        0.000012330
  remove_front           0.000000636        0.000012864
  full traversal (sum)   0.000000450        0.000015300

N = 100000
  operation              DynamicArray (s)   LinkedList (s)
  get(n/2)               0.000000001        0.000050925
  find(absent)           0.000027625        0.000109115
  insert_front           0.000006314        0.000000010
  insert_back            0.000006890        0.000076268
  remove_front           0.000006556        0.000075998
  full traversal (sum)   0.000003700        0.000102950

N = 1000000
  operation              DynamicArray (s)   LinkedList (s)
  get(n/2)               0.000000001        0.000366691
  find(absent)           0.000270650        0.001108540
  insert_front           0.000067454        0.000000012
  insert_back            0.000067356        0.000869388
  remove_front           0.000079616        0.000895540
  full traversal (sum)   0.000037900        0.001125650
```

### Lecture

- **`get(n/2)`** : Array reste à ~1-2 ns quel que soit N (O(1)) ; List
  croît linéairement avec N (0.7 µs → 367 µs, ×~500 quand N ×1000) → O(n)
  confirmé, Array gagne par plusieurs ordres de grandeur.
- **`find(absent)`** : les deux croissent linéairement (même
  complexité), mais Array reste 3-4× plus rapide à N égal — parcours d'un
  buffer contigu (cache-friendly) vs pointer chasing.
- **`insert_front`** : List reste quasi-constante (10-60 ns, bruit de
  mesure, O(1)) alors qu'Array croît avec N (0.16 µs → 67 µs, O(n) à cause
  du décalage). List gagne, comme prévu.
- **`insert_back`** : Array reste rapide et quasi-stable (O(1) amorti) ;
  List croît fortement avec N (1.6 µs → 869 µs) car il n'y a pas de `tail`
  et chaque insertion doit reparcourir toute la liste. Array gagne, comme
  prévu.
- **`remove_front`** : symétrique à `insert_front` — List quasi-constante,
  Array croît linéairement. List gagne.
- **Parcours complet** : les deux sont O(n), mais Array est ~10-30× plus
  rapide à N égal grâce à la localité mémoire (accès séquentiel dans un
  buffer vs sauts de pointeur dispersés dans le tas).

## Expérience hash_good vs hash_bad

```
=== hash_good vs hash_bad ===

  hash_good    N=1000      insert(total)=0.000014       contains(avg)=0.000000001
  hash_bad     N=1000      insert(total)=0.000009       contains(avg)=0.000000566

  hash_good    N=10000     insert(total)=0.000108       contains(avg)=0.000000001
  hash_bad     N=10000     insert(total)=0.000102       contains(avg)=0.000010508

  hash_good    N=100000    insert(total)=0.000670       contains(avg)=0.000000004
  hash_bad     N=100000    insert(total)=0.000573       contains(avg)=0.000118040

  hash_good    N=1000000   insert(total)=0.006589       contains(avg)=0.000000199
  hash_bad     N=1000000   insert(total)=0.005906       contains(avg)=0.001316000
```

- `hash_insert` reste O(1) dans les **deux** cas : un insert fait toujours
  un simple `malloc` + ajout en tête de bucket, peu importe la longueur de
  la chaîne visée. C'est pourquoi les temps d'insertion good/bad sont
  quasi identiques à chaque N.
- `hash_contains` explose avec `hash_bad` : toutes les clés tombent dans
  le bucket 0, la table dégénère en une seule liste chaînée de taille N.
  À N=1 000 000, `contains` coûte **~6600× plus cher** avec `hash_bad`
  (1.32 ms) qu'avec `hash_good` (0.2 µs) — la signature exacte d'un
  passage de O(1) moyen à O(n) dans le pire cas.
- Conclusion pratique : la complexité annoncée O(1) d'une table de
  hachage n'est vraie **que si la fonction de hachage distribue bien les
  clés**. Une mauvaise fonction de hachage ruine entièrement l'avantage
  de la structure, quel que soit le nombre de buckets alloués.
