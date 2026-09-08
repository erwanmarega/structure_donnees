# TP Structures de données — Algorithmique comparative

Implémentation en C de trois structures de données (tableau dynamique, liste
chaînée, table de hachage), comparaison de leur complexité théorique et de
leurs performances mesurées, puis recommandation argumentée selon deux
scénarios contraints.

### Auteur : Erwan Marega

## Structure du projet

| Dossier             | Contenu                                                                                                              |
| ------------------- | -------------------------------------------------------------------------------------------------------------------- |
| [`part_1/`](part_1) | Implémentation des trois structures (DynamicArray, LinkedList, HashTable) + premier benchmark                        |
| [`part_2/`](part_2) | Benchmark complet (get/find/insert_front/insert_back/parcours) + expérience `hash_good` vs `hash_bad`                |
| [`part_3/`](part_3) | Synthèse : tableau de complexités complet, prédictions, résultats, recommandation finale (Situation A / Situation B) |

Chaque dossier a son propre `README.md` détaillé (résultats bruts, méthodologie
de mesure, lecture des chiffres). Ce README racine sert de point d'entrée.

## Compiler et exécuter

```bash
cd part_1 && make run   # structures + benchmark préliminaire
cd part_2 && make run   # benchmark complet + hash_good vs hash_bad
```

`make` seul compile (`cc -Wall -Wextra -O2 -std=c11`), `make clean` supprime
le binaire. `part_3` ne contient pas de code, uniquement l'analyse.

## Résumé des résultats (N = 1 000 000)

| Opération                   | Dynamic Array | Linked List  | Hash Table (good) | Hash Table (bad)         |
| --------------------------- | ------------- | ------------ | ----------------- | ------------------------ |
| `get(n/2)`                  | ~1 ns         | ~367 µs      | N/A               | N/A                      |
| `find`/`contains`           | ~135–270 µs   | ~580–1100 µs | ~0.2–2.6 µs       | ~1.3 ms                  |
| `insert_front`              | ~67–132 µs    | ~10–15 ns    | N/A               | N/A                      |
| `insert_back`/`hash_insert` | ~0.7–1.4 µs   | ~400–870 µs  | ~6.6 µs (total)   | ~5.9 µs (total)          |
| `remove_front`              | ~70–280 µs    | ~8–20 ns     | O(1) moyen        | dégrade comme `contains` |

Détails et lecture complète dans [`part_2/README.md`](part_2/README.md) et
[`part_3/README.md`](part_3/README.md).

## Recommandation finale

- **Situation A** (collection stable, 1 000 000 recherches) → **Hash Table**
  (`hash_good`) : recherche O(1) moyen, coût de construction et mémoire
  amortis sur le volume de recherches.
- **Situation B** (mémoire très limitée, 10 recherches) → **Dynamic Array** :
  le coût fixe des buckets de la Hash Table (~80 Ko) et l'overhead par
  élément de la Linked List ne se justifient pas pour un si faible nombre de
  recherches ; le O(n) d'un `find` sur Array reste négligeable en pratique.

Justification détaillée par critère (temps de recherche, coût de
construction, mémoire, simplicité) dans
[`part_3/README.md`](part_3/README.md#4-recommandation-finale).
