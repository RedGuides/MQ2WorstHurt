---
tags:
  - plugin
resource_link: "https://www.redguides.com/community/resources/mq2worsthurt.82/"
support_link: "https://www.redguides.com/community/threads/mq2worsthurt.66264/"
repository: "https://github.com/RedGuides/MQ2WorstHurt"
authors: "alynel, Sic, Knightly, dannuic"
tagline: "Finds the most injured member of your party, xtar list or pet"
acknowledgements: "toots"
---

# MQ2WorstHurt

<!--desc-start-->
This plugin replaces the forloops to find the worsthurt member of your party, xtar list or pet. You call the TLO with the parameters and it returns a spawns name, which can be manipulated for any usage.
<!--desc-end-->

## Settings

For example, this is all the code I have in my kiss mods to handle all single target healing, and it covers a full xtar list, party members and pets. It is incredibly fast.

```bash
/if ((${Spawn[${WorstHurt[both,1,200,TRUE]}].Type.NotEqual[Pet]} && ${Spawn[${WorstHurt[both,1,200,TRUE]}].Type.NotEqual[Corpse]} && ${Spawn[${WorstHurt[both,1,200,TRUE]}].PctHPs} <= ${Math.Calc[${${Spawn[${WorstHurt[both,1,200,TRUE]}].Class.ShortName}Point}*.${tmpHealOverride}]}) || (${Spawn[${WorstHurt[both,1,200,TRUE]}].Type.Equal[Pet]} && ${Spawn[${WorstHurt[both,1,200,TRUE]}].Type.NotEqual[Corpse]} && ${Spawn[${WorstHurt[both,1,200,TRUE]}].PctHPs} <= ${Math.Calc[${PetPoint}*.${tmpHealOverride}]})) {
        /call newSingleHeal "${Spawn[${WorstHurt[both,1,200,TRUE]}].CleanName}" "${Spawn[${WorstHurt[both,1,200,TRUE]}].Type}" ${Spawn[${WorstHurt[both,1,200,TRUE]}].PctHPs} 1 G
    }
```

This is the syntax,

`${WorstHurt[<group|xtarget|both>,<n>,<radius>,<includePets>]}`

## Top-Level Objects

## [WorstHurt](tlo-worsthurt.md)
{% include-markdown "projects/mq2worsthurt/tlo-worsthurt.md" start="<!--tlo-desc-start-->" end="<!--tlo-desc-end-->" trailing-newlines=false %} {{ readMore('projects/mq2worsthurt/tlo-worsthurt.md') }}

<h2>Forms</h2>
{% include-markdown "projects/mq2worsthurt/tlo-worsthurt.md" start="<!--tlo-forms-start-->" end="<!--tlo-forms-end-->" %}
{% include-markdown "projects/mq2worsthurt/tlo-worsthurt.md" start="<!--tlo-linkrefs-start-->" end="<!--tlo-linkrefs-end-->" %}
