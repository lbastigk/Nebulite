# Nebulite logic file extension .nebl

This language is used for describing and creating state machines for quests and similiar state-transformative operations. The language is currently in ideas phase.

## Example pseudo-code

The following is an example of what the language should be capable of:

``` bash
# Just like bash or python, we use a '#' for comments

# This example is for a simple quest, let's call it deliver-mail

# First, we might wish to include more .nebl files:
# perhaps to:
# - specify overwrites
# - add more states
# - offshoring complex states to separate files
include deliver-mail-addition123.nebl   

# We define the states
domain deliver-mail:
    # using intendation just like with python
    start             # initializer
    talked-to-npc
    obstacle
    delivered
    told-npc # final stage: quest finished
    # for more structurednes, we may wish to create sub-states or just more structure:
    # domain looked-at-mail:
    #   yes
    #   no
    # which we could call with: deliver-mail::looked-at-mail


# each state performs certain actions
state deliver-mail::start:
    # this is the work-in-progress part!
    # the idea is to use this structure to fully build dialogue of npcs
    # as an example, we can then append dialogue to characters if conditions are met
    # all modifiers would be called on actions like:
    # - start of dialogue
    # - start of fight
    # - death of a character
    # - next dialogue
    # - ...
    # Later on it might be helpful to make more specified modifiers, only applying at certain points
    # Currently, the modify is directly parsed in each RenderObject:
    # renderObject.parse("eval if-strcomp {self.name} <QuestGiver> set-dialogue-start ...")
    # Meaning everything after "modify:" is interpreted as Nebulite Script .nebs that we apply to every acter to modify them
    # Which works fine, but might become a burden as we have to apply the if-conditionals over and over again
    # A more streamlined approach is setting standards, and parse them in .nebl language:
    # name QuestGiver:
    #   action <names/attributes>: <command>
    # However, any transformative action (add-option, set position etc...) should be parsed with the .nebs language:
    # action <names/attributes>: <command>
    action on <QuestGiver>: 
        set-dialogue-start "Greetings! ..."
        add-dialogue-option "How can I help you?" "<Answer>" <transformation>
    # We may define special dialogue-modifiers like newline, quoted, newpage etc. in other subsystems

    # Perhaps adding an additional stuff to other npcs:
    # on  - specific name
    # all - specific type
    action all Guards: 
        add-dialogue-rumor "I heard that <QuestGiver> needs help with a parcel delivery"


state deliver-mail::obstacle:
    # Means we are currently in fight with whoever is trying to rob the mail
    # Technically, there is nothing to do here
    # But we may wish to add an extra state: if enemy killed
    # Then we can branch the upcoming dialogue: enemy killed/not killed
    # We may also need more complex logic to check if we ever fought against them:

state deliver-mail::talked-to-npc
    if-happened deliver-mail::obstacle:
        action on <Recipient>: 
            set-dialogue-start "Greetings Traveler. You look worn out. What happened?"
            add-dialogue-option ...
    else:
        action on <Recipient>: 
            set-dialogue-start "Greetings Traveler. Can I help you?"
            add-dialogue-option ...

```