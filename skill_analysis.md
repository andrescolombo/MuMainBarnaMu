# MU Online Class Spells & MU Helper Analysis

This document provides a comprehensive analysis of all character class spells and skills in MU Online, mapping their gameplay classification (**Attack**, **Buff**, or **Heal**), their combat properties (**Ranged/Melee** and **Magic/Physical**), and their behavior in the **MU Helper** automated system.

It specifically highlights the differences between the original upstream repository (`sven-n/MuMain`) and your highly optimized fork (`BarnaMuClient`).

---

## 1. PvP & Player Killing (PK) Rules Explained

If you are experiencing issues where **Player Killing (PK) seems to work sometimes and sometimes not**, or if you feel that some classes can attack you but others cannot, it is due to two core game mechanics: **The Safe Zone Filter** and **The Ctrl-Key Target Lock Requirement**.

### A. The Safe Zone vs. PvP Zone Rule
1. **Towns & Safe Zones:** Inside safe areas (like the center of Lorencia, Noria, Devias, or Elbeland), attacks are completely disabled by both the client and server. No one can hit anyone.
2. **PvP Fields:** Once you step out of the safe zones, PvP is globally enabled.

### B. The Ctrl Key Requirement (Crucial C++ Logic)
In MU Online, to prevent players from accidentally attacking their friends or innocent players during leveling, the client implements a **targeting lock**. 

In your client's C++ source code (`ZzzInterface.cpp` lines 1539, 1778, and 1872), the targeting check is structured like this:
```cpp
if (targetCharacter->PK >= PVP_MURDERER2 || (HIBYTE(GetAsyncKeyState(VK_CONTROL)) == 128 && targetCharacter != Hero))
{
    // Caster is allowed to select and attack the other player!
}
```

#### What this means in practice:
* **Attacking a Murderer (Red Name):** If a player has a PK status of Level 2 (Orange) or Level 3 (Red), **anyone can attack them directly by just clicking on them**. No special keys are required.
* **Attacking a Clean Player (Commoner/Hero):** If a player has a clean name (no PK status), **you CANNOT target or hit them by normal clicking**. To attack them, **you MUST hold down the `Ctrl` key** while casting/clicking.
* **Why it feels like "some classes can kill me and others cannot":**
  1. **AOE Spells vs. Target Spells:** If another player casts a massive area-of-effect (AOE) spell (like *Evil Spirit* or *Lightning Shock*) while holding `Ctrl`, anyone in their vicinity will take damage.
  2. **Elf Buffs / Support:** Supportive spells (like Elf heals) cannot hit you as attacks.
  3. **Range Advantage:** Ranged magic classes can target and kill you from far away before you can close the distance with a Melee class.

---

## 2. Core MU Helper Filtering Logic

The MU Helper setup interface filters and categorizes loaded skills using the following criteria in `CNewUIMuHelperSkillList`:

1. **Master Filter (`PrepareSkillsToRender`):**
   * **Upstream:** If `bySkillUseType == SKILL_USE_TYPE_MASTER` or `SKILL_USE_TYPE_MASTERLEVEL`, the skill is skipped entirely.
   * **Your Fork:** A whitelist exception checks for active Summoner skills. If matched, they pass through this filter even if flagged as master-type in the database.
2. **Category Splitting:**
   * **Heal Skills (`IsHealingSkill`):** Allowed in the primary "Healing Support" quick-slots (elf heal rules).
   * **Buff Skills (`IsBuffSkill`):** Allowed in the passive "Buffs" quick-slots (automatically recast on duration end).
   * **Defense Skills (`IsDefenseSkill`):** Filtered into protective activation slots.
   * **Attack Skills (`IsAttackSkill`):** Anything not categorized as a Buff, Heal, or Defense is designated as an **Attack Skill** (assigned to primary/secondary active combat keys).

---

## 3. Combat Classification & Helper Directory by Class

Below is the complete database of all primary active combat skills for every class, classified by their **Range** (Melee vs. Ranged), **Damage Category** (Magic vs. Physical), and **MU Helper compatibility**.

### 3.1 Dark Wizard (DW) / Soul Master (SM) / Grand Master
* **Damage Rule:** All Dark Wizard attacks deal **Magic Damage** and scale with **Energy**.
* **Combat Style:** Purely **Ranged**.

| Skill Name | Range | Category | In Original Helper? | In Your Fork Helper? | Description / Notes |
| :--- | :---: | :---: | :---: | :---: | :--- |
| Energy Ball | **Ranged** | **Magic** | ✅ Yes | ✅ Yes | Basic starter projectile |
| Fireball | **Ranged** | **Magic** | ✅ Yes | ✅ Yes | Single target projectile |
| Power Wave | **Ranged** | **Magic** | ✅ Yes | ✅ Yes | Ground wave projectile |
| Lightning | **Ranged** | **Magic** | ✅ Yes | ✅ Yes | Strikes and pulls/displaces target |
| Meteor | **Ranged** | **Magic** | ✅ Yes | ✅ Yes | Single target high damage meteor |
| Ice | **Ranged** | **Magic** | ✅ Yes | ✅ Yes | Slows down targets |
| Poison | **Ranged** | **Magic** | ✅ Yes | ✅ Yes | Deals damage over time (DoT) |
| Flame | **Ranged** | **Magic** | ✅ Yes | ✅ Yes | Constant fire stream (close-ranged) |
| Twister | **Ranged** | **Magic** | ✅ Yes | ✅ Yes | Wind cyclone projectile |
| **Evil Spirit** | **Ranged** | **Magic** | ✅ Yes | ✅ Yes | **SM Primary Leveling Spell** (Full AOE) |
| Hell Fire | **Ranged** | **Magic** | ✅ Yes | ✅ Yes | Circular close-range AOE |
| Blast | **Ranged** | **Magic** | ✅ Yes | ✅ Yes | High damage single target |
| Inferno | **Ranged** | **Magic** | ✅ Yes | ✅ Yes | Flame circle around caster |
| Soul Barrier | **Buff** | **Utility** | ✅ Yes | ✅ Yes | **Mana Shield** (goes in Buff slots) |
| Decay | **Ranged** | **Magic** | ✅ Yes | ✅ Yes | High-tier poison meteor (AOE) |
| Ice Storm | **Ranged** | **Magic** | ✅ Yes | ✅ Yes | High-tier ice shower (AOE) |
| Nova | **Ranged** | **Magic** | ✅ Yes | ✅ Yes | Chargeable full-screen AOE |
| Wizardry Exp. | **Buff** | **Utility** | ✅ Yes | ✅ Yes | Increases magic damage (Buff slot) |

---

### 3.2 Dark Knight (DK) / Blade Knight (BK) / Blade Master
* **Damage Rule:** All Dark Knight active attacks deal **Physical Damage** scaling with **Strength** and **Dexterity**.
* **Combat Style:** Purely **Melee** (except *Strike of Destruction*).

| Skill Name | Range | Category | In Original Helper? | In Your Fork Helper? | Description / Notes |
| :--- | :---: | :---: | :---: | :---: | :--- |
| Cyclone | **Melee** | **Physical** | ✅ Yes | ✅ Yes | Sword skill (spinning slash) |
| Slash | **Melee** | **Physical** | ✅ Yes | ✅ Yes | Sword skill (overhead cut) |
| Falling Slash | **Melee** | **Physical** | ✅ Yes | ✅ Yes | Sword skill (leap strike) |
| Lunge | **Melee** | **Physical** | ✅ Yes | ✅ Yes | Sword skill (forward stab) |
| Uppercut | **Melee** | **Physical** | ✅ Yes | ✅ Yes | Sword skill (rising strike) |
| **Twisting Slash** | **Melee** | **Physical** | ✅ Yes | ✅ Yes | **DK Primary Leveling Spell** (AOE circle) |
| Rageful Blow | **Melee** | **Physical** | ✅ Yes | ✅ Yes | Ground slam (close AOE around DK) |
| Death Stab | **Melee** | **Physical** | ✅ Yes | ✅ Yes | High damage linear pierce strike |
| Strike of Destruction | **Ranged** | **Physical** | ✅ Yes | ✅ Yes | Ranged ice shockwave slam |
| Swell Life | **Buff** | **Utility** | ✅ Yes | ✅ Yes | **Greater Fortitude** (HP boost buff) |

---

### 3.3 Fairy Elf (FE) / Muse Elf / High Elf
* **Damage Rule:** All Elf active attacks deal **Physical Damage** and scale with **Dexterity** (for bows) and **Strength**.
* **Combat Style:** Purely **Ranged** (when using Bows/Crossbows).

| Skill Name | Range | Category | In Original Helper? | In Your Fork Helper? | Description / Notes |
| :--- | :---: | :---: | :---: | :---: | :--- |
| Triple Shot | **Ranged** | **Physical** | ✅ Yes | ✅ Yes | Basic multi-arrow bow attack |
| Ice Arrow | **Ranged** | **Physical** | ✅ Yes | ✅ Yes | Freezes targets in place |
| Penetration | **Ranged** | **Physical** | ✅ Yes | ✅ Yes | Linear multi-target piercing arrow |
| Multi Shot | **Ranged** | **Physical** | ✅ Yes | ✅ Yes | Frontal cone multi-arrow AOE |
| Heal | **Heal** | **Utility** | ✅ Yes | ✅ Yes | **Assigned to party/self heal threshold** |
| Greater Defense | **Buff** | **Utility** | ✅ Yes | ✅ Yes | **Defense Buff** (goes in Buff slots) |
| Greater Damage | **Buff** | **Utility** | ✅ Yes | ✅ Yes | **Attack Buff** (goes in Buff slots) |
| Infinity Arrow | **Buff** | **Utility** | ✅ Yes | ✅ Yes | Stops arrow consumption (Buff slot) |

---

### 3.4 Magic Gladiator (MG) / Duel Master
* **Damage Rule:** Can deal **Physical Damage** (scaling with Str/Dex) or **Magic Damage** (scaling with Energy) depending on the equipped weapon (Swords vs. Staffs).

| Skill Name | Range | Category | In Original Helper? | In Your Fork Helper? | Description / Notes |
| :--- | :---: | :---: | :---: | :---: | :--- |
| Power Slash | **Ranged** | **Physical/Magic** | ✅ Yes | ✅ Yes | Weapon projectile sweep |
| Fire Slash | **Melee** | **Physical** | ✅ Yes | ✅ Yes | Close-range sweep (reduces target defense) |
| Gigantic Storm | **Ranged** | **Magic** | ✅ Yes | ✅ Yes | Ranged lightning shower AOE |
| Flame Strike | **Melee** | **Physical** | ✅ Yes | ✅ Yes | Fast frontal fire sweep |
| *DW/DK Spells* | *Varies* | *Varies* | ✅ Yes | ✅ Yes | Can learn SM/BK spells like *Evil Spirit* |

---

### 3.5 Dark Lord (DL) / Lord Emperor
* **Damage Rule:** DL active attacks deal **Physical Damage** and scale with **Strength**, **Dexterity**, and **Command** (for pet/critical boosts).

| Skill Name | Range | Category | In Original Helper? | In Your Fork Helper? | Description / Notes |
| :--- | :---: | :---: | :---: | :---: | :--- |
| Force | **Ranged** | **Physical** | ✅ Yes | ✅ Yes | Single target basic projectile |
| Force Wave | **Ranged** | **Physical** | ✅ Yes | ✅ Yes | Adds weapon damage to Force |
| **Fire Burst** | **Ranged** | **Physical** | ✅ Yes | ✅ Yes | **DL Primary Leveling Spell** (AOE) |
| Earthquake | **Melee** | **Physical** | ✅ Yes | ✅ Yes | Mount stomp (circular AOE + pushback) |
| Thunder Strike | **Melee** | **Physical** | ✅ Yes | ✅ Yes | Spear strike from mount |
| Fire Scream | **Ranged** | **Physical** | ✅ Yes | ✅ Yes | High damage frontal chain fire sweep |
| Critical Damage | **Buff** | **Utility** | ✅ Yes | ✅ Yes | Boosts critical rate/dmg (Buff slot) |

---

### 3.6 Summoner (SU) / Bloody Summoner / Dimension Master
* **Damage Rule:** All Summoner attacks deal **Magic Damage** and scale with **Energy**.
* **Combat Style:** Purely **Ranged**.

| Skill Name | Range | Category | In Original Helper? | In Your Fork Helper? | What Changed & Why? |
| :--- | :---: | :---: | :---: | :---: | :--- |
| **Drain Life** | **Ranged** | **Magic** | ❌ **No** |  **Yes** | **Original:** Treated as a support Elf heal and hidden under Master filters.<br>**Fork:** Correctly classified as an **Attack Spell** and whitelisted! |
| **Chain Lightning** | **Ranged** | **Magic** | ❌ **No** |  **Yes** | **Original:** Hidden as Master level.<br>**Fork:** Whitelisted; targets bounce damage! |
| **Lightning Orb** | **Ranged** | **Magic** | ❌ **No** |  **Yes** | **Original:** Hidden as Master level.<br>**Fork:** Whitelisted; strikes targets from orb! |
| **Berserker** | **Buff** | **Utility** | ❌ **No** |  **Yes** | **Original:** Hidden as Master level.<br>**Fork:** Whitelisted; fits perfectly in Buff slots! |
| **Reflect / Thorns** | **Buff** | **Utility** | ❌ **No** |  **Yes** | **Original:** Hidden as Master level.<br>**Fork:** Whitelisted; fits perfectly in Buff slots! |
| **Sleep / Blind** | **Ranged** | **Magic** | ❌ **No** |  **Yes** | **Original:** Hidden as Master level.<br>**Fork:** Whitelisted; acts as target debuffs! |
| **Weakness / Enervation** | **Ranged** | **Magic** | ❌ **No** |  **Yes** | **Original:** Hidden as Master level.<br>**Fork:** Whitelisted; reduces target stats! |
| Explosion / Requiem | **Ranged** | **Magic** | ✅ Yes | ✅ Yes | standard spell (no master tag in db) |
| Pollution | **Ranged** | **Magic** | ✅ Yes | ✅ Yes | standard spell (no master tag in db) |
| **Lightning Shock** | **Ranged** | **Magic** | ✅ Yes | ✅ Yes | **SU Primary AOE Leveling Spell** |

---

### 3.7 Rage Fighter (RF) / Fist Master
* **Damage Rule:** Attacks deal **Physical Damage** and scale with **Strength** and **Vitality** (unique class trait).
* **Combat Style:** Primarily **Melee** (except *Phoenix Shot*).

| Skill Name | Range | Category | In Original Helper? | In Your Fork Helper? | Description / Notes |
| :--- | :---: | :---: | :---: | :---: | :--- |
| Killing Blow | **Melee** | **Physical** | ✅ Yes | ✅ Yes | Close range basic punch |
| Beast Uppercut | **Melee** | **Physical** | ✅ Yes | ✅ Yes | Close range heavy launcher |
| Chain Drive | **Melee** | **Physical** | ✅ Yes | ✅ Yes | Rapid multi-punch dash |
| **Dark Side** | **Melee** | **Physical** | ✅ Yes | ✅ Yes | **RF Primary Leveling Spell** (Teleports to AOE) |
| Dragon Roar | **Melee** | **Physical** | ✅ Yes | ✅ Yes | Ground stomp circular AOE |
| Dragon Kick | **Melee** | **Physical** | ✅ Yes | ✅ Yes | Frontal kick shockwave sweep |
| Phoenix Shot | **Ranged** | **Physical** | ✅ Yes | ✅ Yes | Ranged fire energy ball projectile |
| Strength/HP/Def Boost | **Buff** | **Utility** | ✅ Yes | ✅ Yes | Increases attack/HP/defense (Buff slots) |
