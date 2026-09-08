# Partie 3 — Décision technique & Rendu

Synthèse des parties [1](../part_1) et [2](../part_2), puis recommandation
argumentée pour deux scénarios contraints.

## 1. Complexités théoriques (tableau complet, 3 structures)

| Opération         | Dynamic Array         | Linked List (sans tail) | Hash Table (chaînage)     |
|--------------------|------------------------|---------------------------|------------------------------|
| `get(index)`       | O(1)                   | O(n)                      | N/A (pas d'accès par position) |
| `find(value)`      | O(n)                   | O(n)                      | O(1) moyen / O(n) pire cas   |
| `insert_front`     | O(n)                   | O(1)                      | N/A (pas de notion d'ordre)  |
| `insert_back`      | O(1) amorti            | O(n)                      | O(1) moyen (`hash_insert`)   |
| `remove_front`     | O(n)                   | O(1)                      | O(1) moyen (`hash_remove`)   |
| Parcours complet   | O(n)                   | O(n)                      | O(TABLE_SIZE + n)            |
| Mémoire / élément  | ~4-8 octets (contigu, capacité doublée) | ~24-32 octets (valeur + pointeur + header malloc) | ~24-32 octets/élément + **80 Ko fixes** (10007 buckets × 8 octets) |

Pire cas Hash Table : mauvaise fonction de hachage → toutes les clés dans un
seul bucket → dégénère en liste chaînée, O(n) (démontré partie 2 avec
`hash_bad`).

## 2. Prédictions avant benchmark

Avant de lancer les mesures (parties 1 et 2), les hypothèses théoriques
étaient :

- **`get(index)`** : Array gagne largement (O(1) vs O(n)) — confirmé.
- **`find`** : même complexité O(n) pour Array et List, mais Array
  prévu plus rapide en pratique grâce à la contiguïté mémoire (cache) —
  confirmé (~3-4× plus rapide).
- **`insert_front`/`remove_front`** : List gagne (O(1) vs O(n)) —
  confirmé.
- **`insert_back`** : Array gagne (O(1) amorti vs O(n), pas de tail sur
  la liste) — confirmé.
- **`hash_contains`** : prévu quasi-instantané tant que le load factor
  reste bas (n ≤ TABLE_SIZE), dégradation attendue au-delà — confirmé
  (temps constant jusqu'à N=10000, puis croissance avec N=100000/1000000).
- **`hash_bad`** : prévu équivalent à une recherche dans une liste
  chaînée simple, donc O(n) — confirmé, ~6600× plus lent que `hash_good`
  à N=1 000 000.

Aucune surprise majeure sur la complexité ; la seule surprise a été
méthodologique (partie 2) : le compilateur `-O2` a effacé une boucle
insert+remove qu'il jugeait sans effet observable, faussant une mesure à
0.000000000 s jusqu'à correction.

## 3. Résultats & comparaison (résumé)

Chiffres détaillés dans [`part_1/README.md`](../part_1/README.md) et
[`part_2/README.md`](../part_2/README.md). Résumé à N=1 000 000 :

| Opération          | DynamicArray | LinkedList  | HashTable (good) | HashTable (bad) |
|---------------------|--------------|-------------|--------------------|--------------------|
| `get(n/2)`          | ~1 ns        | ~367 µs     | N/A                | N/A                |
| `find`/`contains`   | ~135-270 µs  | ~580-1100 µs| ~0.2-2.6 µs        | ~1.3 ms            |
| `insert_front`      | ~67-132 µs   | ~10-15 ns   | N/A                | N/A                |
| `insert_back`/`hash_insert` | ~0.7-1.4 µs | ~400-870 µs | ~6.6 µs (total insert n) | ~5.9 µs (total insert n) |
| `remove_front`      | ~70-280 µs   | ~8-20 ns    | O(1) moyen (non re-mesuré part 2/3) | dégrade comme `contains` |

- **Array** : imbattable pour l'accès indexé et le parcours (localité
  mémoire), mauvais pour les mutations en tête.
- **List** : imbattable pour les mutations en tête (`insert_front`,
  `remove_front` en O(1) garanti), coûteuse pour tout ce qui nécessite un
  parcours (get, find, insert_back).
- **HashTable** : imbattable pour `find`/`contains` sur gros volume
  (jusqu'à ~500× plus rapide que Array à N=100 000), mais seulement si la
  fonction de hachage distribue bien les clés — sinon dégénère
  complètement (démonstration `hash_good` vs `hash_bad`).

## 4. Recommandation finale

### Situation A — collection rarement modifiée, 1 000 000 recherches

**Choix : Hash Table** (avec `hash_good`, chaînage, TABLE_SIZE dimensionné
pour garder un load factor ≤ 1).

Justification par critère :

- **Temps de recherche** : critère dominant ici (1M recherches). Hash
  Table est O(1) moyen contre O(n) pour Array/List. À N=100 000, c'est
  déjà ~500× plus rapide qu'un `find` sur Array (mesures partie 2) ; sur
  1M recherches l'écart cumulé est massif (des heures potentielles de
  différence à grande échelle, secondes contre microsecondes ici même à
  taille modeste).
- **Coût de construction** : payé une seule fois (collection rarement
  modifiée), donc son léger surcoût face à Array (allocations multiples
  au lieu d'un seul buffer) est négligeable amorti sur 1M recherches.
- **Consommation mémoire** : coût fixe de ~80 Ko (buckets) + overhead par
  élément — plus lourd qu'Array, mais acceptable puisque rien n'indique
  de contrainte mémoire dans ce scénario.
- **Simplicité d'implémentation** : plus complexe qu'Array (fonction de
  hachage à soigner — cf. partie 2 sur `hash_bad`), mais le gain de
  recherche justifie largement ce coût ponctuel de dev.

Array serait le choix par défaut si les recherches étaient rares ; ici
elles dominent totalement le profil d'usage, donc Hash Table l'emporte.

### Situation B — mémoire très limitée, seulement 10 recherches

**Choix : Dynamic Array** — je change de structure par rapport à la
Situation A. **Je ne garde pas la Hash Table.**

Justification du compromis mémoire / performance :

- Le coût fixe de la Hash Table (~80 Ko de buckets, indépendant du nombre
  d'éléments réellement stockés) est disproportionné dès que la mémoire
  est contrainte — c'est du gaspillage pur si la collection est petite ou
  si chaque octet compte.
- La Linked List coûte ~24-32 octets par élément (valeur + pointeur +
  overhead d'allocation `malloc`) contre ~4-8 octets par élément pour le
  Dynamic Array (buffer contigu, capacité doublée seulement, pas
  d'overhead par élément). Sur une collection de taille modeste, ce
  facteur ~4-6× compte quand la mémoire est la contrainte principale.
- Avec seulement **10 recherches prévues**, le coût O(n) d'un `find` sur
  Array est négligeable en pratique (même à N=100 000, un `find` coûte
  ~27 µs mesuré partie 2 — 10 fois ça reste ~270 µs, insignifiant). Le
  gain théorique O(1) de la Hash Table ne sert à rien ici : on paierait un
  surcoût mémoire fixe important pour économiser un temps déjà
  négligeable.
- Entre Array et List à mémoire égale de contrainte : Array reste plus
  compact (pas de pointeur ni de header malloc par élément), donc
  préférable à List aussi, sauf si le profil d'usage exige des
  insertions/suppressions fréquentes en tête (non précisé ici).

En résumé : **le nombre de recherches et la contrainte mémoire inversent
le choix** entre les deux situations — la Hash Table gagne quand la
recherche domine le profil d'usage et que la mémoire n'est pas
contrainte (Situation A) ; le Dynamic Array gagne quand la mémoire est la
contrainte dominante et que le volume de recherches ne justifie plus le
surcoût fixe de la table (Situation B).
