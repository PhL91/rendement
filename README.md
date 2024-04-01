# Rendement placement

Petit programme C pour estimer la rentabilité d'un placement.

- Adapté à n'importe quel placement
- Le programme calcule un taux en fonction des dépôts/retraits d'argent dans un placement et en fonction de la valeur finale
- Le taux calculé est un rendement annuel comme si tous les dépôts/retraits avaient été faits à ce taux fixe
- Peut par exemple être utilisé pour calculer le rendement annuel d'un PEA sur une période quelconque: il suffit de rentrer dans le fichier operation les dates et les montants ajoutés au PEA. La valeur finale devra être positionnée à la valeur du PEA à la date choisie.

L'aide:
```
$ ./rendement -h
Utilisation: rendement [OPTION]
  Calcul du rendement d'un placement en fonction des entrées/sorties de capitaux
  -h,--help                             Affichage de l'aide
  -o,--operation <fichier opération>    Défaut: operation.txt
  -d,--date <date de clôture>           Défaut: aujourd'hui
  -v,--valeur <valeur finale>           Défaut: sans mais obligatoire

```
Exemple de lancement:
```
$ ./rendement -v 3805.62 -o test.txt -d 2022-12-31
Nombre d'opérations: 24
Valeur finale:    3805.62
Date d'estimation de la rentabilité: 2022-12-31


                        Nombre de
      Date    Montant    jours de   Intérêts
 opération  opération   placement    générés
2020-01-01     100.00        1095       9.27
2020-02-01     100.00        1064       9.00
2020-03-01     100.00        1035       8.74
2020-04-01     100.00        1004       8.47
2020-05-01     100.00         974       8.21
2020-06-01     100.00         943       7.94
2020-07-01     100.00         913       7.68
2020-08-01     100.00         882       7.41
2020-09-01     100.00         851       7.14
2020-10-01     100.00         821       6.88
2020-11-01     100.00         790       6.61
2020-12-01     100.00         760       6.35
2021-01-01     200.00         729      12.16
2021-02-01     200.00         698      11.63
2021-03-01     200.00         670      11.15
2021-04-01     200.00         639      10.62
2021-05-01     200.00         609      10.11
2021-06-01     200.00         578       9.59
2021-07-01     200.00         548       9.08
2021-08-01     200.00         517       8.55
2021-09-01     200.00         486       8.03
2021-10-01     200.00         456       7.52
2021-11-01     200.00         425       7.00
2021-12-01     200.00         395       6.50

Taux calculé(%):                            3.0005    
Valeur finale après calcul taux:            3805.65   
Nombre d'itérations pour le calcul du taux: 13
```

Exemple de fichier opération:

```
$ cat test.txt
2020-01-01 100
2020-02-01 100
2020-03-01 100
2020-04-01 100
2020-05-01 100
2020-06-01 100
2020-07-01 100
2020-08-01 100
2020-09-01 100
2020-10-01 100
2020-11-01 100
2020-12-01 100
2021-01-01 200
2021-02-01 200
2021-03-01 200
2021-04-01 200
2021-05-01 200
2021-06-01 200
2021-07-01 200
2021-08-01 200
2021-09-01 200
2021-10-01 200
2021-11-01 200
2021-12-01 200
```

## Remarque sur les vérifications effectuées par le programme
En abrégé: très réduite

Plus exactement, il n'y a pas de vérifications exhaustives que les valeurs dans le fichier d'entrée (opération) ne contiennent pas des valeurs aberrantes. Autrement dit, si vous rentrez des valeurs sans réfléchir, le fonctionnement peut s'avérer aléatoire!

## Vérification effectuée
J'ai effectué les opérations dans un tableur et vérifié que j'obtenais les mêmes résultats avec le programme.

