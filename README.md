# MQ2WorstHurt

Replaces the forloops to find the worsthurt member of your party, xtar list or pet. You call the TLO with the parameters and it returns a spawns name, which can be manipulated for any usage.

This plugin was made by alynel, paid for by toots.

## Usage

```txt
${WorstHurt[<group|xtarget|both>,<n>,<radius>,<includePets>]}
```

It will default to both, n = 1, radius = 999999, and includePets = TRUE. So the following are all the same:

```txt
${WorstHurt[both,1,999999,TRUE]}
${WorstHurt[both,1,999999}
${WorstHurt[both,1]}
${WorstHurt[both]}
${WorstHurt}
```

Parameter ordering can be whatever you like, the more often it'll be changed from the default the further to the left it should be to make the macro lines shorter. If you think you're more likely to want to change includePets than radius, for example, they could be swapped.

It returns a spawn type, so you'll have access to all the usual spawn members. Basically if you can do `${Spawn[whatever].Something}`, you can do `${WorstHurt.Something}`.
