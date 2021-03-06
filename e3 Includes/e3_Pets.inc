|--------------------------------------------------------------------------|
|- e3_Macro_Pets.inc v3.0 ( originally designed to work with e3.mac )	    -|
|- Written by Killians of PEQ											                        -|
|--------------------------------------------------------------------------|
|- Checks your pets status. Automatically, resummons, buffs, and shrinks.	-|
|--------------------------------------------------------------------------|
SUB check_Pets
/if (${Debug} || ${Debug_Pets}) /echo |- check_Pets ==>
  |/echo ${Pets} && !${suspend_Pets} && !${combatTimer} && !${activeTimer}
	/if (!${ActionTaken} && !${suspend_Pets} && !${activeTimer}) {
	  /if (${Me.Pet.ID} && ${petHeal2D.Size[1]}) /call check_petHeal
		/if (!${Me.Pet.ID} && !${ActionTaken} && ${petSpell2D.Size[1]} && ${SpawnCount[pc ${Me}]} && (${petSummonCombat} || ${NetBots[${Me}].CombatState} > 0)) /call summonPet
		/if (${Me.Pet.ID} && !${ActionTaken} && ${petBuffs2D.Size[1]} && (${petBuffCombat} || ${NetBots[${Me}].CombatState} > 0)) /call buffPet
		/if (${Me.Pet.ID} && !${ActionTaken} && ${petAutoShrink} && !${combatTimer}) /call check_AutoShrink
	}
/if (${Debug} || ${Debug_Pets}) /echo <== check_Pets -|
/RETURN

|------------------------------------------------------------|
|- Summons a new pet.										                     |
SUB summonPet
/if (${Debug} || ${Debug_Pets}) /echo |- summonPet ==>
  |handle zone delay to prevent summoning pet if one exists
  /if (!${Bool[${Zone.ID}]} || ${currentZone} != ${Zone.ID} || ${SpawnCount[pc = ${Me}]}==0) {
    /delay 3s
    /call check_Zone
    /return
  }
	/call e3_Cast ${Me.ID} "petSpell2D" 1
    /if (${Me.Pet.ID} && ${petTaunt}) {
      /pet taunt on
    } else {
      /pet taunt off
    }
	/if (${Debug} || ${Debug_Pets}) /echo <== summonPet -|
/RETURN

|------------------------------------------|
|- Automatically shrinks pets to min size	-|
SUB check_AutoShrink
/if (${Debug} || ${Debug_Pets}) /echo |- check_AutoShrink ==>
	/if (${Me.Pet.ID} && ${Me.Pet.Height}>1) {
		/if (${FindItemCount[=Gemstone of Dark Flame]}) {
			/casting "Gemstone of Dark Flame" "-targetid|${Me.Pet.ID}"
		} else /if 	(${FindItemCount[=Symbol of Ancient Summoning]}) {
			/casting "Symbol of Ancient Summoning" "-targetid|${Me.Pet.ID}"
		} else /if (${Me.Book[Tiny Companion]}) {
			/casting "Tiny Companion" "-targetid|${Me.Pet.ID}"
		}
		/delay 5s !${Me.Casting.ID}
  }
/if (${Debug} || ${Debug_Pets}) /echo <== check_AutoShrink -|
/RETURN

|-------------------------------------------------|
|- Manually shrinks pets once.	 /bc Shrink Pets -|
#event shrinkPets "<#1#> Shrink Pets"
SUB event_shrinkPets(line, ChatSender)
/if (${Debug} || ${Debug_Pets}) /echo |- event_shrinkPets ==>
	| -Verify the event
	/call verifyEvent "${ChatSender}" "${line}"
	/if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
	|    |-----------Valid User----------|    |--------Included Bot-----------|    |-----------In Zone-------------|
	/if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]} && ${Bool[${Macro.Return.Arg[4,_]}]}) {
		| Attempt to shrink pets.
		/call check_AutoShrink
	}
/if (${Debug} || ${Debug_Pets}) /echo <== event_shrinkPets -|	
/RETURN

|------------------------------------------------------------|
|- Checks pet health, and attempts to cast PetHeal spells.	-|
SUB check_petHeal
/if (${Debug} || ${Debug_Pets}) /echo |- PetHeal ==>
	/if (${Me.Pet.ID}) {
    /declare petMended bool local FALSE
		| Check Mend Companion AA.
		/if (${Me.Pet.PctHPs} < ${petMendPct}) {
		  /if (${Me.AltAbilityReady[Replenish Companion]}) {
        /casting "Replenish Companion" "-targetid|${Me.Pet.ID}"
        /varset petMended TRUE
      } else /if (${Me.AltAbilityReady[Mend Companion]}) {
        /casting "Mend Companion" "-targetid|${Me.Pet.ID}"
        /varset petMended TRUE
      }
		}
		/if (!${petMended}) {
      /declare s int local
      /for s 1 to ${petHeal2D.Size[1]}
        /if (${Me.Pet.PctHPs} <= ${petHeal2D[${s},${iHealPct}]}) {
          /call check_Ready "petHeal2D" ${s}
          /if (${Macro.Return}) {
            /if (${Me.Pet.Distance} < ${petHeal2D[${s},${iMyRange}]}) {
              /call check_Mana "petHeal2D" ${s}
              /if (${Macro.Return}) {
                /call e3_Cast ${Me.Pet.ID} "petHeal2D" ${s}
              }
            }
          }
        }
      /next s
    }
	}
/if (${Debug} || ${Debug_Pets}) /echo <== PetHeal -|
/RETURN

|------------------------------------------------------------------------|
|- Rebuff pet.															                            -|
SUB buffPet
/if (${Debug} || ${Debug_Pets}) /echo |- buffPet ==>
	/declare s int local
	/for s 1 to ${petBuffs2D.Size[1]}
	|/echo ${petBuffs2D[${s},${iCastName}]} stack ${petBuffs2D[${s},${iSpellName}]} ${Spell[${petBuffs2D[${s},${iSpellName}]}].StacksPet} has !${Bool[${Me.PetBuff[${petBuffs2D[${s},${iSpellName}]}]}]}
	|removing ${Spell[${petBuffs2D[${s},${iSpellName}]}].StacksPet} for now as balikor's mark reports non-stacking
  /if (!${Bool[${Me.PetBuff[${petBuffs2D[${s},${iSpellName}]}]}]} && !${Bool[${Me.PetBuff[${petBuffs2D[${s},${iCheckFor}]}]}]}) {
    /call check_Ready "petBuffs2D" ${s}
    /if (${Macro.Return}) {
      /if (${Me.Pet.Distance} < ${petBuffs2D[${s},${iMyRange}]}) {
        /call check_Mana "petBuffs2D" ${s}
        /if (${Macro.Return}) {
          /call e3_Cast ${Me.Pet.ID} "petBuffs2D" ${s}
        }
      }
    }
  }
  :skipBuff
  /if (!${ActionTaken}) /next s
/if (${Debug} || ${Debug_Pets}) /echo <== buffPet -|
/RETURN

|------------------------------------------------------------|
|- Suspend a pet. /bc Suspend Pets
#event suspendMinion "<#1#> Suspend Pets#*#"
SUB event_suspendMinion(live, ChatSender)
  /if (${Debug} || ${Debug_Pets}) /echo |- EVENT_suspendMinion ==>
| -Verify the event
  /call verifyEvent "${ChatSender}" "${line}"
  /if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
|    |-----------Valid User----------|    |--------Included Bot-----------|    |-----------In Zone-------------|
  /if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]} && ${Bool[${Macro.Return.Arg[4,_]}]}) {
    /if (${Me.Pet.ID}) {
      /docommand ${ChatToggle} Suspending my pet...
      /if (${Me.AltAbilityReady[Suspended Minion]}) {
        /if (${Me.Casting.ID}) /call interrupt
        /casting "Suspended Minion|alt" -maxtries 2
        /delay 5s !${Me.Pet.ID}
        /varset currently_Suspended TRUE
      }
      /if (${Me.Pet.ID}) /pet get lost
      /if (${Defined[suspend_Pets]}) /varset suspend_Pets TRUE
    }
  }
  /if (${Debug} || ${Debug_Pets}) /echo <== EVENT_suspendMinion -|
/RETURN

|------------------------------------------------------------|
|- Return suspended pet. /bc return Pets
#event returnMinion "<#1#> return Pets#*#"
SUB event_returnMinion(line, ChatSender)
  /if (${Debug} || ${Debug_Pets}) /echo |- EVENT_returnMinion ==>
| -Verify the event
  /call verifyEvent "${ChatSender}" "${line}"
  /if (${Defined[ChatSender]}) /varset ChatSender ${Macro.Return.Arg[1,_]}
|    |-----------Valid User----------|    |--------Included Bot-----------|    |-----------In Zone-------------|
  /if (${Bool[${Macro.Return.Arg[2,_]}]} && ${Bool[${Macro.Return.Arg[3,_]}]} && ${Bool[${Macro.Return.Arg[4,_]}]}) {
    /if (${suspend_Pets}) {
      /docommand ${ChatToggle} Returning my pet...
      /if (${currently_Suspended}) {
        /casting "Suspended Minion|alt" -maxtries 2
        /varset currently_Suspended FALSE
      }
      /varset suspend_Pets FALSE
    }
  }
  /if (${Debug} || ${Debug_Pets}) /echo <== EVENT_returnMinion -|
/RETURN

|------------------------------------------------------------------------|
|- Setup															                                  -|
Sub pet_Setup
/call INItoVar "${advSettings_Ini},Debug,Debug Pets (On/Off)" Debug_Pets bool outer
/if (${Debug} || ${Debug_Pets}) /echo |- pet_Setup ==>
	/if (${Ini[${Character_Ini},Pets].Length}) {
    |/declare pet_rebuffTimer timer outer
		/if (!${Defined[suspend_Pets]})        /declare suspend_Pets bool global FALSE
		/if (!${Defined[currently_Suspended]}) /declare currently_Suspended bool global FALSE
    /if (${Ini[${Character_Ini},Pets,Pet Mend (Pct)].Length}) /call INItoVar "${Character_Ini},Pets,Pet Mend (Pct)" petMendPct int outer
    /if (${Ini[${Character_Ini},Pets,Pet Taunt (On/Off)].Length}) /call INItoVar "${Character_Ini},Pets,Pet Taunt (On/Off)" petTaunt bool outer
		/if (${Ini[${Character_Ini},Pets,Pet Auto-Shrink (On/Off)].Length}) /call INItoVar "${Character_Ini},Pets,Pet Auto-Shrink (On/Off)" petAutoShrink bool outer
    /if (${Ini[${Character_Ini},Pets,Pet Summon Combat (On/Off)].Length}) /call INItoVar "${Character_Ini},Pets,Summon Combat (On/Off)" petSummonCombat bool outer
    /if (${Ini[${Character_Ini},Pets,Pet Buff Combat (On/Off)].Length}) /call INItoVar "${Character_Ini},Pets,Pet Buff Combat (On/Off)" petBuffCombat bool outer
	}
	/call pet_spellArrays
/if (${Debug} || ${Debug_Pets}) /echo <== pet_Setup -|
/return

|------------------------------------------------------------------------|
|- spell setup
SUB pet_spellArrays
  /if (${Ini[${Character_Ini},Pets,Pet Spell#1].Length}) /call INItoArray "${Character_Ini},Pets,Pet Spell#" petSpell
  /if (${Bool[${petSpell.Size}]}) {
    /call BuildSpellArray "petSpell"	"petSpell2D"
  }
  /if (${Ini[${Character_Ini},Pets,Pet Buff#1].Length}) {
    /call INItoArray "${Character_Ini},Pets,Pet Buff#" petBuffs
    /if (${petBuffs.Size}) /call BuildSpellArray "petBuffs"	"petBuffs2D"
  }
  /if (${Ini[${Character_Ini},Pets,Pet Heal#1].Length}) /call INItoArray "${Character_Ini},Pets,Pet Heal#" petHeal
  /if (${petHeal.Size}) /call BuildSpellArray "petHeal"	"petHeal2D"
/RETURN

|------------------------------------------------------------------------|
SUB pet_MacroSettings
	/call WriteToIni "${advSettings_Ini},Debug,Debug Pets (On/Off)" Off
/RETURN

|------------------------------------------------------------------------|
SUB pet_CharacterSettings
/if (${Debug} || ${Debug_Pets}) /echo |- pet_CharacterSettings ==>
	/if (${Select[${Me.Class.ShortName},BST,ENC,MAG,NEC,SHD,SHM]}) {	
		/call WriteToIni "${Character_Ini},Pets,Pet Spell#1"
    /call WriteToIni "${Character_Ini},Pets,Pet Heal#1"
		/call WriteToIni "${Character_Ini},Pets,Pet Buff#1"
    /if (${Select[${Me.Class.ShortName},BST,MAG,NEC]}) /call WriteToIni "${Character_Ini},Pets,Pet Mend (Pct)"
    /call WriteToIni "${Character_Ini},Pets,Pet Taunt (On/Off)" Off
    /call WriteToIni "${Character_Ini},Pets,Pet Auto-Shrink (On/Off)" Off
    /call WriteToIni "${Character_Ini},Pets,Pet Summon Combat (On/Off)" Off
    /call WriteToIni "${Character_Ini},Pets,Pet Buff Combat (On/Off)" Off
	}
/if (${Debug} || ${Debug_Pets}) /echo <== pet_CharacterSettings -|
/RETURN

|------------------------------------------------------------------------|
Sub pet_Aliases
	/squelch /alias /SuspendPets /bc Suspend Pets
	/squelch /alias /ReturnPets /bc Return Pets
	/squelch /alias /ShrinkPets /bc Shrink Pets
/return

|------------------------------------------------------------------------|
Sub pet_Background_Events
  /doevents suspendMinion
  /doevents returnMinion
  /doevents shrinkPets
/return