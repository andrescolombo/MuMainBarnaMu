# Comprehensive Line-by-Line Changes Registry

This document lists every line and block of code modified or added in the **BarnaMuClient** fork compared to the original upstream **MuMain** repository.

* **Original Upstream Repository:** [sven-n/MuMain](https://github.com/sven-n/MuMain)
* **Active Fork Repository:** [andrescolombo/BarnaMuClient](https://github.com/andrescolombo/BarnaMuClient)

---

## Index of Modified Code Files
1. [src/CMakeLists.txt](#1-srccmakeliststxt)
2. [src/bin/config.ini.template](#2-srcbinconfiginitemplate)
3. [src/source/Data/GameConfig/GameConfigConstants.h](#3-srcsourcedatagameconfiggameconfigconstantsh)
4. [src/source/Engine/Object/ZzzInfomation.cpp](#4-srcsourceengineobjectzzzinfomationcpp)
5. [src/source/Engine/Object/ZzzInventory.cpp](#5-srcsourceengineobjectzzzinventorycpp)
6. [src/source/Engine/Object/ZzzInventory.h](#6-srcsourceengineobjectzzzinventoryh)
7. [src/source/Render/Effects/ZzzEffect.cpp](#7-srcsourcerendereffectszzzeffectcpp)
8. [src/source/Scenes/SceneCore.cpp](#8-srcsourcescenesscenecorecpp)
9. [src/source/UI/NewUI/HUD/NewUIHotKey.cpp](#9-srcsourceuinewuihudnewuihotkeycpp)
10. [src/source/UI/NewUI/HUD/NewUIMiniMap.cpp](#10-srcsourceuinewuihudnewuiminimapcpp)
11. [src/source/UI/NewUI/HUD/NewUIMiniMap.h](#11-srcsourceuinewuihudnewuiminimaph)
12. [src/source/UI/NewUI/Inventory/NewUIInventoryActionController.cpp](#12-srcsourceuinewuiinventorynewuiinventoryactioncontrollercpp)
13. [src/source/UI/NewUI/Inventory/NewUIInventoryCtrl.cpp](#13-srcsourceuinewuiinventorynewuiinventoryctrlcpp)
14. [src/source/UI/NewUI/Inventory/NewUIInventoryCtrl.h](#14-srcsourceuinewuiinventorynewuiinventoryctrlh)
15. [src/source/UI/NewUI/Inventory/NewUIMyInventory.cpp](#15-srcsourceuinewuiinventorynewuimyinventorycpp)
16. [src/source/UI/NewUI/Inventory/NewUIMyInventory.h](#16-srcsourceuinewuiinventorynewuimyinventoryh)
17. [src/source/UI/NewUI/NewUIMuHelper.cpp](#17-srcsourceuinewuinewuimuhelpercpp)

---

## 1. `src/CMakeLists.txt`
* **Change Summary:** Appended environmental tools flag to the .NET Native AOT publishing command to support local compiler toolchains.
* **Line-by-Line Diff:**
```diff
@@ -365,7 +365,7 @@ if (DOTNET_EXECUTABLE)
                   "DOTNET_CLI_HOME=${DOTNET_TEMP_DIR_NATIVE}"
                   "TEMP=${DOTNET_TEMP_DIR_NATIVE}"
                   "TMP=${DOTNET_TEMP_DIR_NATIVE}"
-                  "${DOTNET_EXECUTABLE}" publish "${DOTNET_PROJ_NATIVE}" -c $<CONFIG> -r ${DOTNET_RID} -p:PlatformTarget=${DOTNET_PLATFORM} -o "${DOTNET_TEMP_OUTPUT_NATIVE}" --nologo
+                  "${DOTNET_EXECUTABLE}" publish "${DOTNET_PROJ_NATIVE}" -c $<CONFIG> -r ${DOTNET_RID} -p:PlatformTarget=${DOTNET_PLATFORM} -p:IlcUseEnvironmentalTools=true -o "${DOTNET_TEMP_OUTPUT_NATIVE}" --nologo
           COMMAND ${CMAKE_COMMAND} -E copy_if_different "${DOTNET_TEMP_OUTPUT}/MUnique.Client.Library.dll" "${DOTNET_DLL_PATH}"
           DEPENDS "${DOTNET_PROJ}" ${DOTNET_SOURCES}
           COMMENT "Checking for .NET Client Library updates..."
```

---

## 2. `src/bin/config.ini.template`
* **Change Summary:** Replaced upstream's loopback IP setting with the standard localhost IP.
* **Line-by-Line Diff:**
```diff
@@ -11,5 +11,5 @@ Windowed=1
 SoundVolume=5
 MusicVolume=5
 [CONNECTION SETTINGS]
-ServerIP=127.127.127.127
+ServerIP=127.0.0.1
 ServerPort=44406
```

---

## 3. `src/source/Data/GameConfig/GameConfigConstants.h`
* **Change Summary:** Updated C++ configuration default connection IP to standard localhost.
* **Line-by-Line Diff:**
```diff
@@ -49,8 +49,8 @@ namespace CfgDefaults
     inline constexpr wchar_t CfgDefaultEncryptedUsername[] = L"";
     inline constexpr wchar_t CfgDefaultEncryptedPassword[] = L"";
 
-    inline constexpr wchar_t CfgDefaultServerIP[] = L"127.127.127.127";
+    inline constexpr wchar_t CfgDefaultServerIP[] = L"127.0.0.1";
     inline constexpr int CfgDefaultServerPort = 44406;
 
     inline constexpr int CfgDefaultZoom = 1735;  // OrbitalCamera DEFAULT_RADIUS — matches Default-cam camera-to-Hero distance
-}
\ No newline at end of file
+}
```

---

## 4. `src/source/Engine/Object/ZzzInfomation.cpp`
* **Change Summary:** Fixed compiler warning regarding dynamic item-value rendering formatting by casting the calculated value to integer.
* **Line-by-Line Diff:**
```diff
@@ -435,10 +435,10 @@ void PrintItem(wchar_t* FileName)
                 ItemValue(&ip, 0);
 
                 if (j == 0)
-                    fwprintf(fp, L"%20s %8d %8d %8d %8d %8d %8d %8d %8d %8d\n", p->Name, DamageMin, DamageMax, Defense, SuccessfulBlocking, RequireStrength, RequireDexterity, RequireEnergy, p->WeaponSpeed, ItemValue(&ip));
+                    fwprintf(fp, L"%20s %8d %8d %8d %8d %8d %8d %8d %8d %8d\n", p->Name, DamageMin, DamageMax, Defense, SuccessfulBlocking, RequireStrength, RequireDexterity, RequireEnergy, p->WeaponSpeed, (int)ItemValue(&ip));
                 //fwprintf(fp,"%20s %4d%%",p->Name, iRate);
                 else
-                    fwprintf(fp, L"%17s +%d %8d %8d %8d %8d %8d %8d %8d %8d %8d\n", L"", Level, DamageMin, DamageMax, Defense, SuccessfulBlocking, RequireStrength, RequireDexterity, RequireEnergy, p->WeaponSpeed, ItemValue(&ip));
+                    fwprintf(fp, L"%17s +%d %8d %8d %8d %8d %8d %8d %8d %8d %8d\n", L"", Level, DamageMin, DamageMax, Defense, SuccessfulBlocking, RequireStrength, RequireDexterity, RequireEnergy, p->WeaponSpeed, (int)ItemValue(&ip));
                 //fwprintf(fp,"%4d%%<+%d>",iRate,Level);
             }
             fwprintf(fp, L"\n");
```

---

## 5. `src/source/Engine/Object/ZzzInventory.cpp`
* **Change Summary:**
  1. Configured the tooltip border background colorizer to render colored frames (Red/Yellow) based on item usability status.
  2. Implemented side-by-side comparison tooltips for items, comparing statistics (Defense, Damage, Magic Defense, Block) against active equipment items and coloring values accordingly (Green/Red).
* **Line-by-Line Diff:**
```diff
@@ -298,6 +298,8 @@ int RenderTextList(int sx, int sy, int TextNum, int Tab, int iSort = RT3_SORT_CE
     return TextWidth + Tab;
 }
 
+static TOOLTIP_FRAME_COLOR g_tooltipFrameColor = TOOLTIP_FRAME_NORMAL;
+
 void RenderTipTextList(const int sx, const int sy, int TextNum, int Tab, int iSort, int iRenderPoint, BOOL bUseBG)
 {
     SIZE TextSize = { 0, 0 };
@@ -366,14 +368,21 @@ void RenderTipTextList(const int sx, const int sy, int TextNum, int Tab, int iSo
 
     if (bUseBG == TRUE && TextNum > 0)
     {
-        glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
-        RenderColor((float)iPos_x - 1, fsy - 1, (float)fWidth + 1, (float)1);
-        RenderColor((float)iPos_x - 1, fsy - 1, (float)1, (float)fHeight + 1);
-        RenderColor((float)iPos_x - 1 + fWidth + 1, (float)fsy - 1, (float)1, (float)fHeight + 1);
-        RenderColor((float)iPos_x - 1, fsy - 1 + fHeight + 1, (float)fWidth + 2, (float)1);
-
         glColor4f(0.0f, 0.0f, 0.0f, 0.8f);
         RenderColor((float)iPos_x, fsy, (float)fWidth, (float)fHeight);
+
+        if (g_tooltipFrameColor == TOOLTIP_FRAME_RED)
+            glColor4f(1.0f, 0.15f, 0.15f, 1.0f);
+        else if (g_tooltipFrameColor == TOOLTIP_FRAME_YELLOW)
+            glColor4f(1.0f, 0.85f, 0.0f, 1.0f);
+        else
+            glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
+
+        RenderColor((float)iPos_x - 1, fsy - 1, (float)fWidth + 2, (float)1);
+        RenderColor((float)iPos_x - 1, fsy - 1, (float)1, (float)fHeight + 2);
+        RenderColor((float)iPos_x - 1 + fWidth + 1, (float)fsy - 1, (float)1, (float)fHeight + 2);
+        RenderColor((float)iPos_x - 1, fsy - 1 + fHeight + 1, (float)fWidth + 2, (float)1);
+
         glEnable(GL_TEXTURE_2D);
     }
 
@@ -2106,11 +2115,150 @@ void GetSpecialOptionText(int Type, wchar_t* Text, WORD Option, BYTE Value, int
     }
 }
 
-void RenderItemInfo(int sx, int sy, ITEM* ip, bool Sell, int Inventype, bool bItemTextListBoxUse)
+namespace
+{
+    ITEM* g_pTooltipCompareEquipped = nullptr;
+    constexpr int kCompareTooltipGap = 8;
+
+    float MeasureTipTextListWidth(int TextNum, int Tab = 0)
+    {
+        SIZE TextSize = { 0, 0 };
+        float fWidth = 0.f;
+        for (int i = 0; i < TextNum; ++i)
+        {
+            if (TextList[i][0] == '\0')
+            {
+                break;
+            }
+
+            if (TextBold[i])
+            {
+                g_pRenderText->SetFont(g_hFontBold);
+            }
+            else
+            {
+                g_pRenderText->SetFont(g_hFont);
+            }
+
+            GetTextExtentPoint32(g_pRenderText->GetFontDC(), TextList[i], lstrlen(TextList[i]), &TextSize);
+
+            if (fWidth < TextSize.cx)
+            {
+                fWidth = static_cast<float>(TextSize.cx);
+            }
+        }
+
+        fWidth /= g_fScreenRate_x;
+        if (Tab > 0)
+        {
+            fWidth = Tab / g_fScreenRate_x * 2.f;
+        }
+        return fWidth + 4.f;
+    }
+
+    int ComputeTipLeftFromCenter(int sx, float fWidth)
+    {
+        int iPos_x = sx - static_cast<int>(fWidth / 2.f);
+        if (iPos_x < 0)
+        {
+            iPos_x = 0;
+        }
+        if (iPos_x + fWidth > static_cast<int>(WindowWidth / g_fScreenRate_x))
+        {
+            iPos_x = static_cast<int>(WindowWidth / g_fScreenRate_x - fWidth - 1.f);
+        }
+        return iPos_x;
+    }
+
+    void RenderTipTextListAtLeft(int leftX, int sy, int TextNum, int Tab = 0)
+    {
+        const float fWidth = MeasureTipTextListWidth(TextNum, Tab);
+        int iPos_x = leftX;
+        if (iPos_x < 0)
+        {
+            iPos_x = 0;
+        }
+        if (iPos_x + fWidth > static_cast<int>(WindowWidth / g_fScreenRate_x))
+        {
+            iPos_x = static_cast<int>(WindowWidth / g_fScreenRate_x - fWidth - 1.f);
+        }
+
+        RenderTipTextList(iPos_x + static_cast<int>(fWidth / 2.f), sy, TextNum, Tab);
+    }
+
+    void GetItemAttackRangeForTooltip(const ITEM* ip, int& damageMin, int& damageMax)
+    {
+        damageMin = ip->DamageMin;
+        damageMax = ip->DamageMax;
+
+        if (ip->Level >= ip->Jewel_Of_Harmony_OptionLevel)
+        {
+            StrengthenCapability SC;
+            g_pUIJewelHarmonyinfo->GetStrengthenCapability(&SC, const_cast<ITEM*>(ip), 1);
+
+            if (SC.SI_isSP)
+            {
+                damageMin += SC.SI_SP.SI_minattackpower;
+                damageMax += SC.SI_SP.SI_maxattackpower;
+            }
+        }
+    }
+
+    int GetItemDefenseForTooltip(const ITEM* ip)
+    {
+        int defense = ip->Defense;
+
+        if (ip->Level >= ip->Jewel_Of_Harmony_OptionLevel)
+        {
+            StrengthenCapability SC;
+            g_pUIJewelHarmonyinfo->GetStrengthenCapability(&SC, const_cast<ITEM*>(ip), 2);
+
+            if (SC.SI_isSD)
+            {
+                defense += SC.SI_SD.SI_defense;
+            }
+        }
+
+        return defense;
+    }
+
+    void ApplyTooltipStatCompareColor(int textIndex, int hoverValue, int equippedValue)
+    {
+        if (g_pTooltipCompareEquipped == nullptr || equippedValue <= 0)
+        {
+            return;
+        }
+
+        if (hoverValue > equippedValue)
+        {
+            TextListColor[textIndex] = TEXT_COLOR_GREEN;
+        }
+        else if (hoverValue < equippedValue)
+        {
+            TextListColor[textIndex] = TEXT_COLOR_RED;
+        }
+    }
+}
+
+void SetTooltipFrameColor(TOOLTIP_FRAME_COLOR color)
+{
+    g_tooltipFrameColor = color;
+}
+
+void RenderItemInfo(int sx, int sy, ITEM* ip, bool Sell, int Inventype, bool bItemTextListBoxUse, ITEM* pCompareEquipped, ITEM* pSecondCompareEquipped, bool bAnchorLeft, float* pOutWidth)
 {
     if (ip->Type == -1)
         return;
 
+    if (pCompareEquipped != nullptr)
+    {
+        g_pTooltipCompareEquipped = pCompareEquipped;
+    }
+    else if (!bAnchorLeft)
+    {
+        g_pTooltipCompareEquipped = nullptr;
+    }
+
     tm* ExpireTime;
     if (ip->bPeriodItem == true && ip->bExpiredPeriod == false)
     {
@@ -4002,7 +4150,6 @@ void RenderItemInfo(int sx, int sy, ITEM* ip, bool Sell, int Inventype, bool bIt
             {
                 TextListColor[TextNum] = TEXT_COLOR_YELLOW;
                 TextBold[TextNum] = false;
-                TextNum++;
             }
             else
             {
@@ -4011,8 +4158,17 @@ void RenderItemInfo(int sx, int sy, ITEM* ip, bool Sell, int Inventype, bool bIt
                 else
                     TextListColor[TextNum] = TEXT_COLOR_WHITE;
                 TextBold[TextNum] = false;
-                TextNum++;
             }
+
+            if (g_pTooltipCompareEquipped != nullptr)
+            {
+                int equippedMin = 0;
+                int equippedMax = 0;
+                GetItemAttackRangeForTooltip(g_pTooltipCompareEquipped, equippedMin, equippedMax);
+                ApplyTooltipStatCompareColor(TextNum, DamageMax + maxindex, equippedMax);
+            }
+
+            TextNum++;
         }
         else
         {
@@ -4045,6 +4201,11 @@ void RenderItemInfo(int sx, int sy, ITEM* ip, bool Sell, int Inventype, bool bIt
                 TextListColor[TextNum] = TEXT_COLOR_WHITE;
         }
 
+        if (g_pTooltipCompareEquipped != nullptr)
+        {
+            ApplyTooltipStatCompareColor(TextNum, ip->Defense + maxdefense, GetItemDefenseForTooltip(g_pTooltipCompareEquipped));
+        }
+
         TextBold[TextNum] = false;
         TextNum++;
     }
@@ -4052,6 +4213,7 @@ void RenderItemInfo(int sx, int sy, ITEM* ip, bool Sell, int Inventype, bool bIt
     {
         mu_swprintf(TextList[TextNum], GlobalText[66], ip->MagicDefense);
         TextListColor[TextNum] = TEXT_COLOR_WHITE;
+        ApplyTooltipStatCompareColor(TextNum, ip->MagicDefense, g_pTooltipCompareEquipped ? g_pTooltipCompareEquipped->MagicDefense : 0);
         TextBold[TextNum] = false;
         TextNum++;
     }
@@ -4062,6 +4224,7 @@ void RenderItemInfo(int sx, int sy, ITEM* ip, bool Sell, int Inventype, bool bIt
             TextListColor[TextNum] = TEXT_COLOR_BLUE;
         else
             TextListColor[TextNum] = TEXT_COLOR_WHITE;
+        ApplyTooltipStatCompareColor(TextNum, ip->SuccessfulBlocking, g_pTooltipCompareEquipped ? g_pTooltipCompareEquipped->SuccessfulBlocking : 0);
         TextBold[TextNum] = false;
         TextNum++;
     }
@@ -5637,24 +5800,66 @@ void RenderItemInfo(int sx, int sy, ITEM* ip, bool Sell, int Inventype, bool bIt
 
         int nInvenHeight = p->Height * INVENTORY_SCALE;
 
-        sy += INVENTORY_SCALE;
-        if (sy + Height > iScreenHeight)
+        if (!bAnchorLeft)
         {
-            sy += iScreenHeight - (sy + Height);
+            sy += INVENTORY_SCALE;
+            if (sy + Height > iScreenHeight)
+            {
+                sy += iScreenHeight - (sy + Height);
+            }
+            else if (sy + Height > iScreenHeight)
+            {
+            }
         }
-        else if (sy + Height > iScreenHeight)
+    }
+
+    const float tipWidth = MeasureTipTextListWidth(TextNum, 0);
+
+    if (pOutWidth != nullptr)
+    {
+        *pOutWidth = tipWidth;
+        g_pTooltipCompareEquipped = nullptr;
+        return;
+    }
+
+    const int tipLeft = bAnchorLeft ? sx : ComputeTipLeftFromCenter(sx, tipWidth);
+
+    if (pCompareEquipped != nullptr && pCompareEquipped->Type != -1 && !bAnchorLeft)
+    {
+        // Render main tooltip before compare measurement overwrites TextList.
+        RenderTipTextListAtLeft(tipLeft, sy, TextNum, 0);
+
+        float equippedWidth = 0.f;
+        RenderItemInfo(0, sy, pCompareEquipped, Sell, Inventype, bItemTextListBoxUse, nullptr, nullptr, true, &equippedWidth);
+
+        float secondEquippedWidth = 0.f;
+        if (pSecondCompareEquipped != nullptr && pSecondCompareEquipped->Type != -1)
         {
+            RenderItemInfo(0, sy, pSecondCompareEquipped, Sell, Inventype, bItemTextListBoxUse, nullptr, nullptr, true, &secondEquippedWidth);
         }
-    }
 
-    bool isrendertooltip = true;
+        // Reset frame color so the compare tooltip gets a normal border
+        g_tooltipFrameColor = TOOLTIP_FRAME_NORMAL;
+        g_pTooltipCompareEquipped = nullptr;
+
+        // Render compare tooltip to the LEFT of the main tooltip
+        const int equippedLeft = tipLeft - static_cast<int>(equippedWidth) - kCompareTooltipGap;
+        RenderItemInfo(equippedLeft, sy, pCompareEquipped, Sell, Inventype, bItemTextListBoxUse, nullptr, nullptr, true);
 
-    if (isrendertooltip)
+        if (pSecondCompareEquipped != nullptr && pSecondCompareEquipped->Type != -1)
+        {
+            const int secondEquippedLeft = equippedLeft - static_cast<int>(secondEquippedWidth) - kCompareTooltipGap;
+            RenderItemInfo(secondEquippedLeft, sy, pSecondCompareEquipped, Sell, Inventype, bItemTextListBoxUse, nullptr, nullptr, true);
+        }
+    }
+    else
     {
-        if (bItemTextListBoxUse)
-            RenderTipTextList(sx, sy, TextNum, 0, RT3_SORT_CENTER, STRP_BOTTOMCENTER);
-        else
-            RenderTipTextList(sx, sy, TextNum, 0);
+        RenderTipTextListAtLeft(tipLeft, sy, TextNum, 0);
+        if (!bAnchorLeft)
+        {
+            g_pTooltipCompareEquipped = nullptr;
+            g_tooltipFrameColor = TOOLTIP_FRAME_NORMAL;
+        }
     }
 }
```

---

## 6. `src/source/Engine/Object/ZzzInventory.h`
* **Change Summary:** Extended `RenderItemInfo` signature and added `TOOLTIP_FRAME_COLOR` enum definitions to accommodate tooltip frame painting and comparative arguments.
* **Line-by-Line Diff:**
```diff
@@ -53,7 +53,7 @@ enum SKILL_TOOLTIP_RENDER_POINT
     STRP_BOOTOMRIGHT
 };
 
-// ╝╝└▓┴ñ║©
+// ´┐¢´┐¢´┐¢´┐¢´┐¢´┐¢´┐¢´┐¢
 
 extern int		g_nTaxRate;
 extern int		g_nChaosTaxRate;
@@ -77,7 +77,7 @@ extern int			GuildTotalScore;
 extern int AllRepairGold;
 
 //////////////////////////////////////////////////////////////////////////
-// text ░³À├
+// text ´┐¢´┐¢´┐¢´┐¢
 //////////////////////////////////////////////////////////////////////////
 extern wchar_t TextList[50][100];
 extern int TextListColor[50];
@@ -155,6 +155,9 @@ void RepairAllGold(void);
 WORD CalcMaxDurability(const ITEM* ip, ITEM_ATTRIBUTE* p, int Level);
 void RenderTipTextList(const int sx, const int sy, int TextNum, int Tab, int iSort = RT3_SORT_CENTER, int iRenderPoint = STRP_NONE, BOOL bUseBG = TRUE);
 
+enum TOOLTIP_FRAME_COLOR { TOOLTIP_FRAME_NORMAL = 0, TOOLTIP_FRAME_RED, TOOLTIP_FRAME_YELLOW };
+void SetTooltipFrameColor(TOOLTIP_FRAME_COLOR color);
+
 void SendRequestUse(int Index, int Target, bool addPoints = true);
 bool SendRequestEquipmentItem(STORAGE_TYPE iSrcType, int iSrcIndex, ITEM* pItem, STORAGE_TYPE iDstType, int iDstIndex);
 bool IsCanUseItem();
@@ -178,7 +181,7 @@ bool GetAttackDamage(int* iMinDamage, int* iMaxDamage);
 void GetItemName(int iType, int iLevel, wchar_t* Text);
 std::wstring GetItemDisplayName(ITEM* pItem);
 void GetSpecialOptionText(int Type, wchar_t* Text, WORD Option, BYTE Value, int iMana);
-void RenderItemInfo(int sx, int sy, ITEM* ip, bool Sell, int Inventype = 0, bool bItemTextListBoxUse = false);
+void RenderItemInfo(int sx, int sy, ITEM* ip, bool Sell, int Inventype = 0, bool bItemTextListBoxUse = false, ITEM* pCompareEquipped = nullptr, ITEM* pSecondCompareEquipped = nullptr, bool bAnchorLeft = false, float* pOutWidth = nullptr);
 void RenderRepairInfo(int sz, int sy, ITEM* ip, bool Sell);
 void RenderSkillInfo(int sx, int sy, int Type, int SkillNum = 0, int iRenderPoint = STRP_NONE);
 void RequireClass(ITEM_ATTRIBUTE* p);
```

---

## 7. `src/source/Render/Effects/ZzzEffect.cpp`
* **Change Summary:** Expanded `arv3PosProcess` position array size from 3 to 4 elements to fix a critical buffer overflow index out-of-bounds error on the target trajectory rendering path.
* **Line-by-Line Diff:**
```diff
@@ -5156,7 +5156,7 @@ void CreateEffect(int Type, vec3_t Position, vec3_t Angle, vec3_t Light, int Sub
                 {
                     const int	TOTAL_LIFETIME = 60;
                     vec3_t		v3PosStart, v3PosTarget;
-                    vec3_t		arv3PosProcess[3];
+                    vec3_t		arv3PosProcess[4];
 
                     o->ExtState = TOTAL_LIFETIME;
                     o->LifeTime = TOTAL_LIFETIME;
```

---

## 8. `src/source/Scenes/SceneCore.cpp`
* **Change Summary:** Changed standard Game Client initial connection server IP default to local loopback.
* **Line-by-Line Diff:**
```diff
@@ -53,7 +53,7 @@ short   g_shCameraLevel = 0;
 
 int g_iLengthAuthorityCode = 20;
 
-const wchar_t* szServerIpAddress = L"127.127.127.127";
+const wchar_t* szServerIpAddress = L"127.0.0.1";
 WORD g_ServerPort = 44406;
 
 EGameScene  SceneFlag = WEBZEN_SCENE;
```

---

## 9. `src/source/UI/NewUI/HUD/NewUIHotKey.cpp`
* **Change Summary:** Implemented key binding for the `F8` hotkey to toggle starting and stopping the Mu Helper bot instantly.
* **Line-by-Line Diff:**
```diff
@@ -342,6 +343,12 @@ bool SEASON3B::CNewUIHotKey::UpdateKeyEvent()
         PlayBuffer(SOUND_CLICK01);
         return false;
     }
+    else if (SEASON3B::IsPress(VK_F8))
+    {
+        MUHelper::g_MuHelper.Toggle();
+        PlayBuffer(SOUND_CLICK01);
+        return false;
+    }
     return true;
 }
```

---

## 10. `src/source/UI/NewUI/HUD/NewUIMiniMap.cpp`
* **Change Summary:**
  1. Implemented **Minimap click-to-move** pathfinding navigation. Handles unrotated coordinate calculation (correcting axis directions), cell walkability, and continuous movement segmented path updates.
  2. Integrated GM / Administrator **instant teleportation on right-click** inside the minimap coordinate frame.
  3. Built custom **Spawn Point parser and rendering system** to load coordinate markers from local map text files (`SpawnPoints_[MapName].txt`) and render them with distinctive colored palette tags.
* **Line-by-Line Diff:**
```diff
@@ -14,14 +14,84 @@
 #include "UI/NewUI/Inventory/NewUIMyInventory.h"
 #include "GameLogic/Items/CSItemOption.h"
 #include "World/MapInfra/MapManager.h"
+#include "Engine/AI/ZzzAI.h"
+#include "Engine/Object/ZzzCharacter.h"
+#include "Engine/Object/ZzzInterface.h"
+#include "Render/Effects/ZzzEffect.h"
+#include "Render/Terrain/ZzzLodTerrain.h"
+#include "Network/Server/WSclient.h"
 
 extern BYTE m_OccupationState;
+extern int TargetX;
+extern int TargetY;
 
 using namespace SEASON3B;
 
+namespace
+{
+    constexpr int MINIMAP_VIEWPORT_X = 0;
+    constexpr int MINIMAP_VIEWPORT_Y = 0;
+    constexpr int MINIMAP_VIEWPORT_WIDTH = REFERENCE_WIDTH;
+    constexpr int MINIMAP_VIEWPORT_HEIGHT = 430;
+    constexpr int MINIMAP_TILE_MIN = 0;
+    constexpr int MINIMAP_TILE_MAX = TERRAIN_SIZE - 1;
+    constexpr int MINIMAP_ZOOM_LEVEL_COUNT = 6;
+    constexpr float MINIMAP_PLAYER_CENTER_X = static_cast<float>(REFERENCE_WIDTH) * 0.5f;
+    constexpr float MINIMAP_PLAYER_CENTER_Y = static_cast<float>(REFERENCE_HEIGHT) * 0.5f;
+    constexpr float MINIMAP_INVERSE_ROTATION_FACTOR = 0.70710678f;
+    constexpr float MINIMAP_TILE_COUNT = static_cast<float>(TERRAIN_SIZE);
+    constexpr float MINIMAP_TARGET_EFFECT_SCALE = 0.6f;
+    constexpr float MINIMAP_TARGET_LIGHT = 1.0f;
+
+    int ClampMinimapTile(int value)
+    {
+        if (value < MINIMAP_TILE_MIN)
+        {
+            return MINIMAP_TILE_MIN;
+        }
+        if (value > MINIMAP_TILE_MAX)
+        {
+            return MINIMAP_TILE_MAX;
+        }
+
+        return value;
+    }
+
+    bool IsMinimapTileWalkable(int tileX, int tileY)
+    {
+        const WORD attribute = TerrainWall[TERRAIN_INDEX(tileX, tileY)];
+        return attribute < TW_NOGROUND
+            || (attribute & TW_ACTION) == TW_ACTION
+            || (attribute & TW_HEIGHT) == TW_HEIGHT;
+    }
+
+    void CreateMinimapMoveTargetEffect(int tileX, int tileY)
+    {
+        OBJECT* pHeroObj = &Hero->Object;
+        vec3_t targetPosition;
+        vec3_t targetLight;
+
+        targetPosition[0] = static_cast<float>(tileX) * TERRAIN_SCALE + (TERRAIN_SCALE * 0.5f);
+        targetPosition[1] = static_cast<float>(tileY) * TERRAIN_SCALE + (TERRAIN_SCALE * 0.5f);
+        targetPosition[2] = RequestTerrainHeight(targetPosition[0], targetPosition[1]);
+
+        Vector(MINIMAP_TARGET_LIGHT, MINIMAP_TARGET_LIGHT, 0.0f, targetLight);
+        DeleteEffect(MODEL_MOVE_TARGETPOSITION_EFFECT);
+
+        if ((TerrainWall[TERRAIN_INDEX(tileX, tileY)] & TW_NOMOVE) != TW_NOMOVE)
+        {
+            CreateEffect(MODEL_MOVE_TARGETPOSITION_EFFECT, targetPosition, pHeroObj->Angle, targetLight,
+                0, pHeroObj, -1, 0, 0, 0, MINIMAP_TARGET_EFFECT_SCALE);
+        }
+    }
+}
+
 SEASON3B::CNewUIMiniMap::CNewUIMiniMap()
 {
     m_pNewUIMng = NULL;
+    m_HasMoveTarget = false;
+    m_MoveTargetX = 0;
+    m_MoveTargetY = 0;
 }
 
 SEASON3B::CNewUIMiniMap::~CNewUIMiniMap()
@@ -69,6 +139,7 @@ bool SEASON3B::CNewUIMiniMap::Create(CNewUIManager* pNewUIMng, int x, int y)
 
 void SEASON3B::CNewUIMiniMap::ClosingProcess()
 {
+    ResetMoveTarget();
     SocketClient->ToGameServer()->SendCloseNpcRequest();
 }
 
@@ -173,6 +244,8 @@ bool SEASON3B::CNewUIMiniMap::Render()
             break;
     }
 
+    Render_Spawns();
+
     float Ch_wid = 12;
     RenderImage(IMAGE_MINIMAP_INTERFACE + 3, 325, 230, Ch_wid, Ch_wid, 0.f, 0.f, 17.5f / 32.f, 17.5f / 32.f);
 
@@ -202,6 +275,7 @@ bool SEASON3B::CNewUIMiniMap::Render()
 
 bool SEASON3B::CNewUIMiniMap::Update()
 {
+    ContinueMoveTarget();
     return true;
 }
 
@@ -276,42 +350,253 @@ void SEASON3B::CNewUIMiniMap::LoadImages(const wchar_t* Filename)
 
         delete[] Buffer;
     }
+
+    LoadSpawnPoints(Filename);
 }
 
 void SEASON3B::CNewUIMiniMap::UnloadImages()
 {
     DeleteBitmap(IMAGE_MINIMAP_INTERFACE);
+    m_SpawnPoints.clear();
 }
 
-bool SEASON3B::CNewUIMiniMap::UpdateMouseEvent()
+void SEASON3B::CNewUIMiniMap::LoadSpawnPoints(const wchar_t* worldName)
+{
+    m_SpawnPoints.clear();
+
+    wchar_t path[300];
+    mu_swprintf(path, L"Data\\Local\\%ls\\Minimap\\SpawnPoints_%ls.txt",
+                g_strSelectedML.c_str(), worldName);
+
+    FILE* fp = _wfopen(path, L"r");
+    if (fp == nullptr)
+        return;
+
+    char line[256];
+    while (fgets(line, static_cast<int>(sizeof(line)), fp) != nullptr)
+    {
+        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r')
+            continue;
+
+        SpawnPoint sp;
+        char nameUtf8[64] = {};
+        const int matched = sscanf_s(line, "%d %d %63[^\n\r]",
+                                     &sp.locX, &sp.locY,
+                                     nameUtf8, static_cast<unsigned>(sizeof(nameUtf8)));
+        if (matched < 2)
+            continue;
+
+        if (matched >= 3)
+            MultiByteToWideChar(CP_UTF8, 0, nameUtf8, -1, sp.name, static_cast<int>(sizeof(sp.name) / sizeof(sp.name[0])));
+        else
+            sp.name[0] = L'\0';
+
+        m_SpawnPoints.push_back(sp);
+    }
+    fclose(fp);
+}
+
+void SEASON3B::CNewUIMiniMap::Render_Spawns()
 {
-    bool ret = true;
+    if (m_SpawnPoints.empty() || Hero == nullptr)
+        return;
 
+    static constexpr float kPalette[][3] = {
+        { 1.f, 0.3f, 0.3f },   // red
+        { 1.f, 0.7f, 0.1f },   // orange
+        { 0.3f, 1.f, 0.3f },   // green
+        { 0.4f, 0.8f, 1.f },   // cyan
+        { 0.9f, 0.4f, 1.f },   // purple
+        { 1.f, 1.f, 0.2f },    // yellow
+    };
+    constexpr int kPaletteSize = static_cast<int>(sizeof(kPalette) / sizeof(kPalette[0]));
+
+    const float zoomWidth  = static_cast<float>(m_Lenth[m_MiniPos].x);
+    const float zoomHeight = static_cast<float>(m_Lenth[m_MiniPos].y);
+    const float heroTx = (static_cast<float>(Hero->PositionY) / 256.f) * zoomWidth;
+    const float heroTy = (static_cast<float>(Hero->PositionX) / 256.f) * zoomHeight;
+
+    constexpr float kRot      = 45.f;
+    constexpr float kUvScale  = 17.5f / 32.f;
+    constexpr int   kMarkSize = 12;
+
+    for (int i = 0; i < static_cast<int>(m_SpawnPoints.size()); ++i)
+    {
+        const SpawnPoint& sp = m_SpawnPoints[i];
+        const float* col = kPalette[i % kPaletteSize];
+        glColor4f(col[0], col[1], col[2], 1.f);
+
+        const float spTx = (static_cast<float>(sp.locY) / 256.f) * zoomWidth;
+        const float spTy = (static_cast<float>(sp.locX) / 256.f) * zoomHeight;
+        RenderPointRotate(IMAGE_MINIMAP_INTERFACE + 5, spTx, spTy,
+                          kMarkSize, kMarkSize,
+                          zoomWidth - heroTx, zoomHeight - heroTy,
+                          zoomWidth, zoomHeight,
+                          kRot, 0.f, kUvScale, kUvScale, -1);
+    }
+    glColor4f(1.f, 1.f, 1.f, 1.f);
+}
+
+bool SEASON3B::CNewUIMiniMap::UpdateMouseEvent()
+{
     if (m_BtnExit.UpdateMouseEvent() == true)
     {
         g_pNewUISystem->Hide(SEASON3B::INTERFACE_MINI_MAP);
         return true;
     }
 
-    if (IsPress(VK_LBUTTON))
+    if (IsPress(VK_LBUTTON) &&
+        CheckMouseIn(MINIMAP_VIEWPORT_X, MINIMAP_VIEWPORT_Y, MINIMAP_VIEWPORT_WIDTH, MINIMAP_VIEWPORT_HEIGHT))
+    {
+        if (Check_Mouse(MouseX, MouseY))
+        {
+            PlayBuffer(SOUND_CLICK01);
+        }
+
+        return false;
+    }
+
+    static bool s_isAdminLatched = false;
+    if (Hero != nullptr)
+    {
+        const bool isAdminNow =
+            (Hero->CtlCode & (CTLCODE_08OPERATOR | CTLCODE_20OPERATOR)) != 0
+            || g_isCharacterBuff((&Hero->Object), eBuff_GMEffect) != FALSE;
+        if (isAdminNow)
+        {
+            s_isAdminLatched = true;
+        }
+    }
+
+    if (IsPress(VK_RBUTTON) &&
+        CheckMouseIn(MINIMAP_VIEWPORT_X, MINIMAP_VIEWPORT_Y, MINIMAP_VIEWPORT_WIDTH, MINIMAP_VIEWPORT_HEIGHT) &&
+        s_isAdminLatched)
     {
-        ret = Check_Mouse(MouseX, MouseY);
-        if (ret == false)
+        const float zoomWidth  = static_cast<float>(m_Lenth[m_MiniPos].x);
+        const float zoomHeight = static_cast<float>(m_Lenth[m_MiniPos].y);
+        if (m_bSuccess && Hero && zoomWidth > 0.f && zoomHeight > 0.f)
         {
+            const float screenOffsetX = static_cast<float>(MouseX) - MINIMAP_PLAYER_CENTER_X;
+            const float screenOffsetY = MINIMAP_PLAYER_CENTER_Y - static_cast<float>(MouseY);
+            const float mapHorizontalOffset = (screenOffsetX + screenOffsetY) * MINIMAP_INVERSE_ROTATION_FACTOR;
+            const float mapVerticalOffset   = (-screenOffsetX + screenOffsetY) * MINIMAP_INVERSE_ROTATION_FACTOR;
+
+            const int targetTileY = ClampMinimapTile(static_cast<int>(
+                Hero->PositionY + mapHorizontalOffset * (MINIMAP_TILE_COUNT / zoomWidth)));
+            const int targetTileX = ClampMinimapTile(static_cast<int>(
+                Hero->PositionX - mapVerticalOffset   * (MINIMAP_TILE_COUNT / zoomHeight)));
+
+            SocketClient->ToGameServer()->SendInstantMoveRequest(
+                static_cast<BYTE>(targetTileX), static_cast<BYTE>(targetTileY));
+            CreateMinimapMoveTargetEffect(targetTileX, targetTileY);
             PlayBuffer(SOUND_CLICK01);
         }
+        return false;
     }
 
-    if (CheckMouseIn(0, 0, REFERENCE_WIDTH, 430))
+    if (CheckMouseIn(MINIMAP_VIEWPORT_X, MINIMAP_VIEWPORT_Y, MINIMAP_VIEWPORT_WIDTH, MINIMAP_VIEWPORT_HEIGHT))
+    {
+        return false;
+    }
+
+    return true;
+}
+
+void SEASON3B::CNewUIMiniMap::ResetMoveTarget()
+{
+    m_HasMoveTarget = false;
+    m_MoveTargetX = 0;
+    m_MoveTargetY = 0;
+}
+
+bool SEASON3B::CNewUIMiniMap::TrySendMoveSegment(int targetTileX, int targetTileY)
+{
+    if (Hero == nullptr)
+    {
+        return false;
+    }
+    if (Hero->PositionX == targetTileX && Hero->PositionY == targetTileY)
+    {
+        return false;
+    }
+    if (!PathFinding2(Hero->PositionX, Hero->PositionY, targetTileX, targetTileY, &Hero->Path))
+    {
+        return false;
+    }
+
+    TargetX = targetTileX;
+    TargetY = targetTileY;
+    Hero->MovementType = MOVEMENT_MOVE;
+    SendMove(Hero, &Hero->Object);
+    return true;
+}
+
+void SEASON3B::CNewUIMiniMap::ContinueMoveTarget()
+{
+    if (!m_HasMoveTarget)
+    {
+        return;
+    }
+    if (Hero == nullptr)
+    {
+        ResetMoveTarget();
+        return;
+    }
+    if (Hero->PositionX == m_MoveTargetX && Hero->PositionY == m_MoveTargetY)
+    {
+        ResetMoveTarget();
+        return;
+    }
+    if (Hero->Movement && Hero->Path.CurrentPath < Hero->Path.PathNum - 1)
+    {
+        return;
+    }
+    if (!TrySendMoveSegment(m_MoveTargetX, m_MoveTargetY))
+    {
+        ResetMoveTarget();
+    }
+}
+
+bool SEASON3B::CNewUIMiniMap::Check_Mouse(int mx, int my)
+{
+    if (m_bSuccess == false || Hero == nullptr)
+    {
+        return false;
+    }
+    if (m_MiniPos < 0 || m_MiniPos >= MINIMAP_ZOOM_LEVEL_COUNT)
+    {
+        return false;
+    }
+
+    const float zoomWidth = static_cast<float>(m_Lenth[m_MiniPos].x);
+    const float zoomHeight = static_cast<float>(m_Lenth[m_MiniPos].y);
+    if (zoomWidth <= 0.0f || zoomHeight <= 0.0f)
+    {
+        return false;
+    }
+
+    const float screenOffsetX = static_cast<float>(mx) - MINIMAP_PLAYER_CENTER_X;
+    const float screenOffsetY = MINIMAP_PLAYER_CENTER_Y - static_cast<float>(my);
+    const float mapHorizontalOffset = (screenOffsetX + screenOffsetY) * MINIMAP_INVERSE_ROTATION_FACTOR;
+    const float mapVerticalOffset = (-screenOffsetX + screenOffsetY) * MINIMAP_INVERSE_ROTATION_FACTOR;
+
+    const int targetTileY = ClampMinimapTile(static_cast<int>(
+        Hero->PositionY + mapHorizontalOffset * (MINIMAP_TILE_COUNT / zoomWidth)));
+    const int targetTileX = ClampMinimapTile(static_cast<int>(
+        Hero->PositionX - mapVerticalOffset * (MINIMAP_TILE_COUNT / zoomHeight)));
+    if (!IsMinimapTileWalkable(targetTileX, targetTileY))
+    {
+        return false;
+    }
+    if (!TrySendMoveSegment(targetTileX, targetTileY))
+    {
+        return false;
+    }
+
+    m_HasMoveTarget = true;
+    m_MoveTargetX = targetTileX;
+    m_MoveTargetY = targetTileY;
+    CreateMinimapMoveTargetEffect(targetTileX, targetTileY);
+    return true;
 }
```

---

## 11. `src/source/UI/NewUI/HUD/NewUIMiniMap.h`
* **Change Summary:** Declared custom variables (`m_SpawnPoints`, `m_HasMoveTarget`, `m_MoveTargetX/Y`) and member functions to support path navigation and spawn locations.
* **Line-by-Line Diff:**
```diff
@@ -4,6 +4,7 @@
 
 #pragma once
 
+#include <vector>
 #include "UI/NewUI/NewUIBase.h"
 #include "UI/NewUI/NewUIManager.h"
 #include "UI/NewUI/HUD/NewUIMainFrameWindow.h"
@@ -20,6 +21,13 @@ namespace SEASON3B
             IMAGE_MINIMAP_INTERFACE = BITMAP_MINI_MAP_BEGIN,
         };
 
+        struct SpawnPoint
+        {
+            int     locX;
+            int     locY;
+            wchar_t name[64];
+        };
+
         enum MASTER_DATA
         {
             SKILL_ICON_DATA_WDITH = 4,
@@ -50,6 +58,10 @@ namespace SEASON3B
         CNewUIButton			m_BtnExit;
         MINI_MAP				m_Mini_Map_Data[MAX_MINI_MAP_DATA];
         float					m_Btn_Loc[MAX_MINI_MAP_DATA][4];
+        bool					m_HasMoveTarget;
+        int						m_MoveTargetX;
+        int						m_MoveTargetY;
+        std::vector<SpawnPoint>	m_SpawnPoints;
 
     public:
         bool					m_bSuccess;
@@ -80,6 +92,11 @@ namespace SEASON3B
         void Render_Text();
         void Render_Icon();
         void Render_Scroll();
+        void Render_Spawns();
+        void LoadSpawnPoints(const wchar_t* worldName);
+        void ResetMoveTarget();
+        bool TrySendMoveSegment(int targetTileX, int targetTileY);
+        void ContinueMoveTarget();
         bool Check_Mouse(int mx, int my);
         bool Check_Btn(int mx, int my);
     };
```

---

## 12. `src/source/UI/NewUI/Inventory/NewUIInventoryActionController.cpp`
* **Change Summary:** Re-ordered action checks within the inventory Right-Click controller. Displaced extension tab moves downstream so right-clicking an equipable item triggers quick-equipping FIRST instead of incorrectly executing tab transitions first.
* **Line-by-Line Diff:**
```diff
@@ -254,11 +254,6 @@ bool CNewUIInventoryActionController::HandleSellToNPC(CNewUIInventoryCtrl* targe
 
 bool CNewUIInventoryActionController::HandleInventoryRightClickActions(CNewUIInventoryCtrl* targetControl) const
 {
-    if (g_pNewUISystem->IsVisible(INTERFACE_INVENTORY_EXT))
-    {
-        return TryTransferBetweenInventorySections(targetControl);
-    }
-
     ITEM* pItem = targetControl->FindItemAtPt(MouseX, MouseY);
     if (pItem == nullptr)
     {
@@ -301,6 +296,11 @@ bool CNewUIInventoryActionController::HandleInventoryRightClickActions(CNewUIInv
         }
     }
 
+    if (g_pNewUISystem->IsVisible(INTERFACE_INVENTORY_EXT))
+    {
+        return TryTransferBetweenInventorySections(targetControl);
+    }
+
     if (TryDropItem(targetControl, pItem))
     {
         return true;
```

---

## 13. `src/source/UI/NewUI/Inventory/NewUIInventoryCtrl.cpp`
* **Change Summary:**
  1. Implemented transparent red/yellow backdrops (`RenderEquipStatusBackgrounds`) on items that cannot be equipped due to class restrictions or deficient stat requirements.
  2. Upgraded tooltip generation to recursively invoke compared equipment tooltips side-by-side.
* **Line-by-Line Diff:**
```diff
@@ -5,6 +5,7 @@
 #include "stdafx.h"
 #include "UI/NewUI/Inventory/NewUIInventoryCtrl.h"
 #include "UI/NewUI/Inventory/NewUIItemMng.h"
+#include "UI/NewUI/Inventory/NewUIMyInventory.h"
 #include "UI/NewUI/NewUISystem.h"
 #include "Engine/Object/ZzzInventory.h"
 #include "GameLogic/Items/CComGem.h"
@@ -13,8 +14,182 @@
 #include "Network/Server/SocketSystem.h"
 #include "World/MapInfra/MapManager.h"
 #include "GameLogic/Items/MixMgr.h"
+#include "Character/CharacterManager.h"
 using namespace SEASON3B;
 
+namespace
+{
+    constexpr int MAX_COMPARE_EQUIPPED_ITEMS = 2;
+    constexpr float EQUIP_STATUS_BACKGROUND_ALPHA = 0.12f;
+
+    enum class EquipStatus { None, ClassMismatch, StatInsufficient, CanEquip };
+
+    // Class + step-class check only (no slot, no stat requirements).
+    bool IsClassCompatibleForItem(const ITEM* pItem)
+    {
+        const ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pItem->Type];
+        const BYTE byFirstClass = gCharacterManager.GetBaseClass(Hero->Class);
+        const BYTE byStepClass  = gCharacterManager.GetStepClass(Hero->Class);
+
+        bool bClassOk = pItemAttr->RequireClass[byFirstClass] != 0;
+        if (!bClassOk && byFirstClass == CLASS_DARK
+            && pItemAttr->RequireClass[CLASS_WIZARD] && pItemAttr->RequireClass[CLASS_KNIGHT])
+            bClassOk = true;
+
+        if (!bClassOk)
+            return false;
+        if (pItemAttr->RequireClass[byFirstClass] > byStepClass)
+            return false;
+
+        return true;
+    }
+
+    // Stat + level check only (no class, no slot).
+    bool HasSufficientStatsForItem(const ITEM* pItem)
+    {
+        const WORD wStrength  = CharacterAttribute->Strength  + CharacterAttribute->AddStrength;
+        const WORD wDexterity = CharacterAttribute->Dexterity + CharacterAttribute->AddDexterity;
+        const WORD wEnergy    = CharacterAttribute->Energy    + CharacterAttribute->AddEnergy;
+        const WORD wVitality  = CharacterAttribute->Vitality  + CharacterAttribute->AddVitality;
+        const WORD wCharisma  = CharacterAttribute->Charisma  + CharacterAttribute->AddCharisma;
+        const WORD wLevel     = CharacterAttribute->Level;
+
+        if (pItem->RequireStrength  > wStrength)  return false;
+        if (pItem->RequireDexterity > wDexterity) return false;
+        if (pItem->RequireEnergy    > wEnergy)    return false;
+        if (pItem->RequireVitality  > wVitality)  return false;
+        if (pItem->RequireCharisma  > wCharisma)  return false;
+        if (pItem->RequireLevel     > wLevel)     return false;
+
+        return true;
+    }
+
+    // Returns equip status for tooltip frame coloring.
+    EquipStatus GetEquipStatus(const ITEM* pItem)
+    {
+        if (pItem == nullptr || pItem->Type == -1)
+            return EquipStatus::None;
+        if (Hero == nullptr || CharacterAttribute == nullptr)
+            return EquipStatus::None;
+
+        const ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pItem->Type];
+        const int equipSlot = pItemAttr->m_byItemSlot;
+        if (equipSlot < 0 || equipSlot >= MAX_EQUIPMENT_INDEX)
+            return EquipStatus::None;
+
+        if (!IsClassCompatibleForItem(pItem))
+            return EquipStatus::ClassMismatch;
+
+        if (!HasSufficientStatsForItem(pItem))
+            return EquipStatus::StatInsufficient;
+
+        return EquipStatus::CanEquip;
+    }
+
+    // Class+slot check without stat requirements so stat-insufficient items
+    // still show the side-by-side compare tooltip.
+    bool IsEquipableIgnoreStats(int equipSlot, const ITEM* pItem)
+    {
+        if (!IsClassCompatibleForItem(pItem))
+            return false;
+
+        const ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pItem->Type];
+        if (pItemAttr->m_byItemSlot == equipSlot)
+            return true;
+        if (pItemAttr->m_byItemSlot == EQUIPMENT_WEAPON_RIGHT && equipSlot == EQUIPMENT_WEAPON_LEFT)
+            return true;
+        if (pItemAttr->m_byItemSlot == EQUIPMENT_RING_RIGHT && equipSlot == EQUIPMENT_RING_LEFT)
+            return true;
+
+        return false;
+    }
+
+    bool CanCompareEquippedSlot(int equipSlot, const ITEM* pHoverItem)
+    {
+        if (equipSlot < 0 || equipSlot >= MAX_EQUIPMENT_INDEX)
+        {
+            return false;
+        }
+        if (CharacterMachine->Equipment[equipSlot].Type == -1)
+        {
+            return false;
+        }
+        if (g_pMyInventory->IsEquipable(equipSlot, const_cast<ITEM*>(pHoverItem)))
+        {
+            return true;
+        }
+
+        return IsEquipableIgnoreStats(equipSlot, const_cast<ITEM*>(pHoverItem));
+    }
+
+    void AddEquippedCompareSlot(const ITEM* pHoverItem, int equipSlot, int* pSlots, int& slotCount)
+    {
+        if (slotCount >= MAX_COMPARE_EQUIPPED_ITEMS || !CanCompareEquippedSlot(equipSlot, pHoverItem))
+        {
+            return;
+        }
+
+        for (int i = 0; i < slotCount; ++i)
+        {
+            if (pSlots[i] == equipSlot)
+            {
+                return;
+            }
+        }
+
+        pSlots[slotCount++] = equipSlot;
+    }
+
+    int GetEquippedCompareSlots(const ITEM* pHoverItem, int* pSlots)
+    {
+        if (pHoverItem == nullptr || pHoverItem->Type == -1 || g_pMyInventory == nullptr)
+        {
+            return 0;
+        }
+
+        const ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pHoverItem->Type];
+        const int equipSlot = pItemAttr->m_byItemSlot;
+        if (equipSlot < 0 || equipSlot >= MAX_EQUIPMENT_INDEX)
+        {
+            return 0;
+        }
+
+        int slotCount = 0;
+        if (equipSlot == EQUIPMENT_WEAPON_RIGHT)
+        {
+            AddEquippedCompareSlot(pHoverItem, EQUIPMENT_WEAPON_LEFT, pSlots, slotCount);
+            AddEquippedCompareSlot(pHoverItem, EQUIPMENT_WEAPON_RIGHT, pSlots, slotCount);
+        }
+        else if (equipSlot == EQUIPMENT_RING_RIGHT)
+        {
+            AddEquippedCompareSlot(pHoverItem, EQUIPMENT_RING_LEFT, pSlots, slotCount);
+            AddEquippedCompareSlot(pHoverItem, EQUIPMENT_RING_RIGHT, pSlots, slotCount);
+        }
+        else
+        {
+            AddEquippedCompareSlot(pHoverItem, equipSlot, pSlots, slotCount);
+        }
+
+        return slotCount;
+    }
+
+    bool SetEquipStatusBackgroundColor(EquipStatus status)
+    {
+        if (status == EquipStatus::ClassMismatch)
+        {
+            glColor4f(1.0f, 0.15f, 0.15f, EQUIP_STATUS_BACKGROUND_ALPHA);
+            return true;
+        }
+        if (status == EquipStatus::StatInsufficient)
+        {
+            glColor4f(1.0f, 0.85f, 0.0f, EQUIP_STATUS_BACKGROUND_ALPHA);
+            return true;
+        }
+
+        return false;
+    }
+}
+
 SEASON3B::CNewUIPickedItem::CNewUIPickedItem()
 {
     m_pNewItemMng = nullptr;
@@ -1207,6 +1382,7 @@ void SEASON3B::CNewUIInventoryCtrl::Render()
 
     if (m_pNew3DRenderMng)
     {
+        RenderEquipStatusBackgrounds();
         m_pNew3DRenderMng->RenderUI2DEffect(INVENTORY_CAMERA_Z_ORDER, UI2DEffectCallback, this, RENDER_NUMBER_OF_ITEM, 0);
         if (m_pToolTipItem && GetPickedItem() == nullptr)
         {
@@ -1506,6 +1682,39 @@ void SEASON3B::CNewUIInventoryCtrl::RenderNumberOfItem()
     DisableAlphaBlend();
 }
 
+void SEASON3B::CNewUIInventoryCtrl::RenderEquipStatusBackgrounds()
+{
+    if (m_StorageType != STORAGE_TYPE::INVENTORY)
+    {
+        return;
+    }
+    if (Hero == nullptr || CharacterAttribute == nullptr)
+    {
+        return;
+    }
+
+    EnableAlphaTest();
+
+    for (ITEM* pItem : m_vecItem)
+    {
+        const EquipStatus status = GetEquipStatus(pItem);
+        if (!SetEquipStatusBackgroundColor(status))
+        {
+            continue;
+        }
+
+        const ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pItem->Type];
+        const float x = m_Pos.x + (pItem->x * INVENTORY_SQUARE_WIDTH);
+        const float y = m_Pos.y + (pItem->y * INVENTORY_SQUARE_HEIGHT);
+        const float width = pItemAttr->Width * INVENTORY_SQUARE_WIDTH;
+        const float height = pItemAttr->Height * INVENTORY_SQUARE_HEIGHT;
+
+        RenderColor(x, y, width, height);
+    }
+
+    EndRenderColor();
+}
+
 void SEASON3B::CNewUIInventoryCtrl::RenderItemToolTip()
 {
     if (m_pToolTipItem)
@@ -1521,7 +1730,25 @@ void SEASON3B::CNewUIInventoryCtrl::RenderItemToolTip()
 
         if (m_ToolTipType == TOOLTIP_TYPE_INVENTORY)
         {
-            RenderItemInfo(iTargetX, iTargetY, m_pToolTipItem, false);
+            ITEM* pEquippedCompare = nullptr;
+            ITEM* pSecondEquippedCompare = nullptr;
+            if (m_StorageType == STORAGE_TYPE::INVENTORY)
+            {
+                int equipSlots[MAX_COMPARE_EQUIPPED_ITEMS] = { -1, -1 };
+                const int equipSlotCount = GetEquippedCompareSlots(m_pToolTipItem, equipSlots);
+                if (equipSlotCount > 0)
+                {
+                    pEquippedCompare = &CharacterMachine->Equipment[equipSlots[0]];
+                }
+                if (equipSlotCount > 1)
+                {
+                    pSecondEquippedCompare = &CharacterMachine->Equipment[equipSlots[1]];
+                }
+
+                SetTooltipFrameColor(TOOLTIP_FRAME_NORMAL);
+            }
+
+            RenderItemInfo(iTargetX, iTargetY, m_pToolTipItem, false, 0, false, pEquippedCompare, pSecondEquippedCompare);
         }
         else if (m_ToolTipType == TOOLTIP_TYPE_REPAIR)
         {
```

---

## 14. `src/source/UI/NewUI/Inventory/NewUIInventoryCtrl.h`
* **Change Summary:** Declared `RenderEquipStatusBackgrounds` within inventory controller definitions.
* **Line-by-Line Diff:**
```diff
@@ -274,6 +274,7 @@ namespace SEASON3B
         //protected:
         void Render3D();
         void RenderNumberOfItem();
+        void RenderEquipStatusBackgrounds();
         void RenderItemToolTip();
     };
 }
```

---

## 15. `src/source/UI/NewUI/Inventory/NewUIMyInventory.cpp`
* **Change Summary:**
  1. Developed **Auto-Rearrange Inventory button & packing algorithm**. Generates a sorting layout plan (prioritizing larger dimension items), calculates collision movement stages, and sequentially executes packets to the game server to shift bag contents without client de-sync.
  2. Placed the custom `m_BtnArrange` button frame inside the active HUD inventory drawer, managing state registration.
* **Line-by-Line Diff:**
```diff
@@ -34,8 +34,476 @@ extern bool SelectFlag;
 #include "Audio/DSPlaySound.h"
 #include "Engine/Object/ZzzInterface.h"
 
+#include <algorithm>
+#include <vector>
+
 using namespace SEASON3B;
 
+namespace
+{
+    constexpr int ARRANGE_BUTTON_X_OFFSET = 161;
+    constexpr int ARRANGE_BUTTON_Y_OFFSET = 391;
+    constexpr int ARRANGE_BUTTON_WIDTH = 18;
+    constexpr int ARRANGE_BUTTON_HEIGHT = 29;
+
+    struct ArrangeItem
+    {
+        DWORD key;
+        int sourceIndex;
+        int sourceX;
+        int sourceY;
+        int targetX;
+        int targetY;
+        int width;
+        int height;
+    };
+
+    struct ArrangePlacementScore
+    {
+        int area;
+        int bottom;
+        int right;
+        int row;
+        int column;
+        int columnLane;
+        int oddColumn;
+    };
+
+    bool IsInventoryRearrangeInterfaceBlocked()
+    {
+        if (g_pNewUISystem == nullptr)
+        {
+            return true;
+        }
+
+        return g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPCSHOP) == true
+            || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_TRADE) == true
+            || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_DEVILSQUARE) == true
+            || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_BLOODCASTLE) == true
+            || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MIXINVENTORY) == true
+            || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_STORAGE) == true
+            || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MYSHOP_INVENTORY) == true
+            || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_LUCKYITEMWND) == true
+            || g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_PURCHASESHOP_INVENTORY) == true;
+    }
+
+    int GetArrangeIndex(int column, int row, int columnCount)
+    {
+        return row * columnCount + column;
+    }
+
+    int GetArrangeArea(const ArrangeItem& item)
+    {
+        return item.width * item.height;
+    }
+
+    bool IsArrangeItemAtTarget(const ArrangeItem& item)
+    {
+        return item.sourceX == item.targetX && item.sourceY == item.targetY;
+    }
+
+    bool IsAllowedArrangeItemSize(int width, int height)
+    {
+        return width >= 1 && width <= 4 && height >= 1 && height <= 4;
+    }
+
+    void OccupyArrangeCells(std::vector<DWORD>& occupied, int columnCount, const ArrangeItem& item, DWORD value)
+    {
+        for (int y = 0; y < item.height; ++y)
+        {
+            for (int x = 0; x < item.width; ++x)
+            {
+                occupied[GetArrangeIndex(item.sourceX + x, item.sourceY + y, columnCount)] = value;
+            }
+        }
+    }
+
+    bool CanPlaceArrangeItem(const std::vector<DWORD>& occupied, int columnCount, int rowCount, const ArrangeItem& item)
+    {
+        if (item.targetX + item.width > columnCount || item.targetY + item.height > rowCount)
+        {
+            return false;
+        }
+
+        for (int y = 0; y < item.height; ++y)
+        {
+            for (int x = 0; x < item.width; ++x)
+            {
+                const DWORD cellKey = occupied[GetArrangeIndex(item.targetX + x, item.targetY + y, columnCount)];
+                if (cellKey != 0 && cellKey != item.key)
+                {
+                    return false;
+                }
+            }
+        }
+
+        return true;
+    }
+
+    bool CanPlaceArrangeTarget(const std::vector<bool>& occupied, int columnCount, int rowCount, const ArrangeItem& item, int column, int row)
+    {
+        if (column + item.width > columnCount || row + item.height > rowCount)
+        {
+            return false;
+        }
+
+        for (int y = 0; y < item.height; ++y)
+        {
+            for (int x = 0; x < item.width; ++x)
+            {
+                if (occupied[GetArrangeIndex(column + x, row + y, columnCount)])
+                {
+                    return false;
+                }
+            }
+        }
+
+        return true;
+    }
+
+    bool IsBetterArrangePlacement(const ArrangeItem& item, const ArrangePlacementScore& candidate, const ArrangePlacementScore& best)
+    {
+        if (item.width > 1)
+        {
+            if (candidate.oddColumn != best.oddColumn)
+            {
+                return candidate.oddColumn < best.oddColumn;
+            }
+            if (candidate.columnLane != best.columnLane)
+            {
+                return candidate.columnLane < best.columnLane;
+            }
+            if (candidate.row != best.row)
+            {
+                return candidate.row < best.row;
+            }
+        }
+
+        if (candidate.bottom != best.bottom)
+        {
+            return candidate.bottom < best.bottom;
+        }
+        if (candidate.area != best.area)
+        {
+            return candidate.area < best.area;
+        }
+        if (candidate.right != best.right)
+        {
+            return candidate.right < best.right;
+        }
+        if (candidate.row != best.row)
+        {
+            return candidate.row < best.row;
+        }
+
+        return candidate.column < best.column;
+    }
+
+    bool FindBestArrangeTarget(const std::vector<bool>& occupied, int columnCount, int rowCount, ArrangeItem& item)
+    {
+        bool found = false;
+        ArrangePlacementScore bestScore{};
+        int bestColumn = 0;
+        int bestRow = 0;
+
+        for (int row = 0; row < rowCount; ++row)
+        {
+            for (int column = 0; column < columnCount; ++column)
+            {
+                if (!CanPlaceArrangeTarget(occupied, columnCount, rowCount, item, column, row))
+                {
+                    continue;
+                }
+
+                const int bottom = row + item.height;
+                const int right = column + item.width;
+                const ArrangePlacementScore candidateScore = {
+                    bottom * right,
+                    bottom,
+                    right,
+                    row,
+                    column,
+                    column / item.width,
+                    column % item.width
+                };
+
+                if (!found || IsBetterArrangePlacement(item, candidateScore, bestScore))
+                {
+                    found = true;
+                    bestScore = candidateScore;
+                    bestColumn = column;
+                    bestRow = row;
+                }
+            }
+        }
+
+        if (!found)
+        {
+            return false;
+        }
+
+        item.targetX = bestColumn;
+        item.targetY = bestRow;
+        return true;
+    }
+
+    void OccupyArrangeTargetCells(std::vector<bool>& occupied, int columnCount, const ArrangeItem& item)
+    {
+        for (int y = 0; y < item.height; ++y)
+        {
+            for (int x = 0; x < item.width; ++x)
+            {
+                occupied[GetArrangeIndex(item.targetX + x, item.targetY + y, columnCount)] = true;
+            }
+        }
+    }
+
+    void MarkArrangeTargetCells(std::vector<bool>& targetCells, int columnCount, const ArrangeItem& item)
+    {
+        for (int y = 0; y < item.height; ++y)
+        {
+            for (int x = 0; x < item.width; ++x)
+            {
+                targetCells[GetArrangeIndex(item.targetX + x, item.targetY + y, columnCount)] = true;
+            }
+        }
+    }
+
+    DWORD FindArrangeTargetBlocker(const std::vector<DWORD>& occupied, int columnCount, const ArrangeItem& item)
+    {
+        for (int y = 0; y < item.height; ++y)
+        {
+            for (int x = 0; x < item.width; ++x)
+            {
+                const DWORD cellKey = occupied[GetArrangeIndex(item.targetX + x, item.targetY + y, columnCount)];
+                if (cellKey != 0 && cellKey != item.key)
+                {
+                    return cellKey;
+                }
+            }
+        }
+
+        return 0;
+    }
+
+    ArrangeItem* FindArrangeItemByKey(std::vector<ArrangeItem>& items, DWORD key)
+    {
+        for (ArrangeItem& item : items)
+        {
+            if (item.key == key)
+            {
+                return &item;
+            }
+        }
+
+        return nullptr;
+    }
+
+    bool CanPlaceArrangeTemporary(const std::vector<DWORD>& occupied, const std::vector<bool>& targetCells, int columnCount, int rowCount, const ArrangeItem& item, int column, int row, bool avoidTargetCells)
+    {
+        if (column + item.width > columnCount || row + item.height > rowCount)
+        {
+            return false;
+        }
+        if (column == item.sourceX && row == item.sourceY)
+        {
+            return false;
+        }
+
+        for (int y = 0; y < item.height; ++y)
+        {
+            for (int x = 0; x < item.width; ++x)
+            {
+                const int index = GetArrangeIndex(column + x, row + y, columnCount);
+                if (avoidTargetCells && targetCells[index])
+                {
+                    return false;
+                }
+                if (occupied[index] != 0 && occupied[index] != item.key)
+                {
+                    return false;
+                }
+            }
+        }
+
+        return true;
+    }
+
+    bool FindArrangeTemporaryTarget(const std::vector<DWORD>& occupied, const std::vector<bool>& targetCells, int columnCount, int rowCount, ArrangeItem& item, int& targetX, int& targetY)
+    {
+        for (int pass = 0; pass < 2; ++pass)
+        {
+            const bool avoidTargetCells = pass == 0;
+            for (int row = rowCount - item.height; row >= 0; --row)
+            {
+                for (int column = columnCount - item.width; column >= 0; --column)
+                {
+                    if (CanPlaceArrangeTemporary(occupied, targetCells, columnCount, rowCount, item, column, row, avoidTargetCells))
+                    {
+                        targetX = column;
+                        targetY = row;
+                        return true;
+                    }
+                }
+            }
+        }
+
+        return false;
+    }
+
+    bool ComputeArrangeTargets(std::vector<ArrangeItem>& items, int columnCount, int rowCount)
+    {
+        std::sort(items.begin(), items.end(), [](const ArrangeItem& left, const ArrangeItem& right)
+        {
+            const int leftArea = GetArrangeArea(left);
+            const int rightArea = GetArrangeArea(right);
+            if (leftArea != rightArea)
+            {
+                return leftArea > rightArea;
+            }
+            if (left.height != right.height)
+            {
+                return left.height > right.height;
+            }
+            if (left.width != right.width)
+            {
+                return left.width > right.width;
+            }
+            return left.key < right.key;
+        });
+
+        std::vector<bool> occupied(columnCount * rowCount, false);
+
+        for (ArrangeItem& item : items)
+        {
+            if (!FindBestArrangeTarget(occupied, columnCount, rowCount, item))
+            {
+                return false;
+            }
+
+            OccupyArrangeTargetCells(occupied, columnCount, item);
+        }
+
+        return true;
+    }
+
+    void MoveArrangeItem(std::vector<DWORD>& occupied, int columnCount, ArrangeItem& item, int targetX, int targetY, std::vector<SEASON3B::InventoryRearrangeMove>& moves)
+    {
+        OccupyArrangeCells(occupied, columnCount, item, 0);
+        item.sourceX = targetX;
+        item.sourceY = targetY;
+        OccupyArrangeCells(occupied, columnCount, item, item.key);
+
+        moves.push_back({ item.key, targetY * columnCount + targetX + MAX_EQUIPMENT });
+    }
+
+    void ReplaceArrangeMoveItemKey(std::vector<SEASON3B::InventoryRearrangeMove>& moves, DWORD oldKey, DWORD newKey)
+    {
+        if (oldKey == newKey)
+        {
+            return;
+        }
+
+        for (SEASON3B::InventoryRearrangeMove& move : moves)
+        {
+            if (move.itemKey == oldKey)
+            {
+                move.itemKey = newKey;
+            }
+        }
+    }
+
+    bool GenerateArrangeMoves(std::vector<ArrangeItem> items, int columnCount, int rowCount, std::vector<SEASON3B::InventoryRearrangeMove>& moves)
+    {
+        std::sort(items.begin(), items.end(), [columnCount](const ArrangeItem& left, const ArrangeItem& right)
+        {
+            const int leftTargetIndex = left.targetY * columnCount + left.targetX;
+            const int rightTargetIndex = right.targetY * columnCount + right.targetX;
+            if (leftTargetIndex != rightTargetIndex)
+            {
+                return leftTargetIndex < rightTargetIndex;
+            }
+
+            return left.key < right.key;
+        });
+
+        std::vector<DWORD> occupied(columnCount * rowCount, 0);
+        std::vector<bool> targetCells(columnCount * rowCount, false);
+        for (const ArrangeItem& item : items)
+        {
+            OccupyArrangeCells(occupied, columnCount, item, item.key);
+            MarkArrangeTargetCells(targetCells, columnCount, item);
+        }
+
+        while (true)
+        {
+            if (moves.size() > items.size() * columnCount * rowCount)
+            {
+                return false;
+            }
+
+            bool hasPendingItem = false;
+            bool movedItem = false;
+
+            for (ArrangeItem& item : items)
+            {
+                if (IsArrangeItemAtTarget(item))
+                {
+                    continue;
+                }
+
+                hasPendingItem = true;
+                if (!CanPlaceArrangeItem(occupied, columnCount, rowCount, item))
+                {
+                    continue;
+                }
+
+                MoveArrangeItem(occupied, columnCount, item, item.targetX, item.targetY, moves);
+                movedItem = true;
+                break;
+            }
+
+            if (!hasPendingItem)
+            {
+                return true;
+            }
+            if (!movedItem)
+            {
+                for (ArrangeItem& item : items)
+                {
+                    if (IsArrangeItemAtTarget(item))
+                    {
+                        continue;
+                    }
+
+                    const DWORD blockerKey = FindArrangeTargetBlocker(occupied, columnCount, item);
+                    ArrangeItem* pBlocker = FindArrangeItemByKey(items, blockerKey);
+                    if (pBlocker == nullptr)
+                    {
+                        continue;
+                    }
+
+                    int temporaryX = 0;
+                    int temporaryY = 0;
+                    if (!FindArrangeTemporaryTarget(occupied, targetCells, columnCount, rowCount, *pBlocker, temporaryX, temporaryY))
+                    {
+                        continue;
+                    }
+
+                    MoveArrangeItem(occupied, columnCount, *pBlocker, temporaryX, temporaryY, moves);
+                    movedItem = true;
+                    break;
+                }
+
+                if (!movedItem)
+                {
+                    return false;
+                }
+            }
+        }
+    }
+}
+
 CNewUIMyInventory::CNewUIMyInventory()
 {
     m_pNewUIMng = nullptr;
@@ -52,6 +520,8 @@ CNewUIMyInventory::CNewUIMyInventory()
 
     m_bRepairEnableLevel = false;
     m_bMyShopOpen = false;
+    m_bInventoryRearrangePending = false;
+    m_bInventoryRearrangeMoveInFlight = false;
 }
 
 CNewUIMyInventory::~CNewUIMyInventory()
@@ -421,6 +891,7 @@ void CNewUIMyInventory::SetPos(int x, int y)
     m_BtnRepair.SetPos(m_Pos.x + 50, m_Pos.y + 391);
     m_BtnMyShop.SetPos(m_Pos.x + 87, m_Pos.y + 391);
     m_BtnExpand.SetPos(m_Pos.x + 87 + 37, m_Pos.y + 391);
+    m_BtnArrange.SetPos(m_Pos.x + ARRANGE_BUTTON_X_OFFSET, m_Pos.y + ARRANGE_BUTTON_Y_OFFSET);
 }
 
 const POINT& CNewUIMyInventory::GetPos() const
@@ -684,6 +1155,8 @@ bool CNewUIMyInventory::Update()
 
     if (IsVisible())
     {
+        ProcessInventoryRearrange();
+
         m_iPointedSlot = -1;
         for (int i = 0; i < MAX_EQUIPMENT_INDEX; i++)
         {
@@ -839,6 +1312,9 @@ void CNewUIMyInventory::OpenningProcess()
 
 void CNewUIMyInventory::ClosingProcess()
 {
+    m_bInventoryRearrangePending = false;
+    m_bInventoryRearrangeMoveInFlight = false;
+    m_InventoryRearrangeMoves.clear();
     m_pNewInventoryCtrl->BackupPickedItem();
     RepairEnable = 0;
     SetRepairMode(false);
@@ -1202,6 +1678,10 @@ void CNewUIMyInventory::SetButtonInfo()
     m_BtnExpand.ChangeButtonImgState(true, IMAGE_INVENTORY_EXPAND_BTN, false);
     m_BtnExpand.ChangeButtonInfo(m_Pos.x + 87 + 37, m_Pos.y + 391, 36, 29);
     m_BtnExpand.ChangeToolTipText(GlobalText[3322], true);
+
+    m_BtnArrange.ChangeButtonImgState(true, IMAGE_INVENTORY_EXPAND_BTN, false);
+    m_BtnArrange.ChangeButtonInfo(m_Pos.x + ARRANGE_BUTTON_X_OFFSET, m_Pos.y + ARRANGE_BUTTON_Y_OFFSET, ARRANGE_BUTTON_WIDTH, ARRANGE_BUTTON_HEIGHT);
+    m_BtnArrange.ChangeToolTipText(L"Rearrange inventory", true);
 }
 
 void CNewUIMyInventory::LoadImages() const
@@ -1371,6 +1851,7 @@ void CNewUIMyInventory::RenderButtons()
         {
             m_BtnMyShop.Render();
         }
+        m_BtnArrange.Render();
     }
     m_BtnExit.Render();
     m_BtnExpand.Render();
@@ -1632,11 +2113,207 @@ bool CNewUIMyInventory::BtnProcess()
 
             return true;
         }
+
+        if (m_BtnArrange.UpdateMouseEvent() == true)
+        {
+            RequestInventoryRearrange();
+            return true;
+        }
     }
 
     return false;
 }
 
+bool CNewUIMyInventory::CanUseInventoryRearrange() const
+{
+    return m_pNewInventoryCtrl != nullptr
+        && CNewUIInventoryCtrl::GetPickedItem() == nullptr
+        && EquipmentItem == false
+        && IsInventoryRearrangeInterfaceBlocked() == false;
+}
+
+bool CNewUIMyInventory::BuildInventoryRearrangeMoves(std::vector<InventoryRearrangeMove>& moves) const
+{
+    if (m_pNewInventoryCtrl == nullptr)
+    {
+        return false;
+    }
+
+    moves.clear();
+    const int columnCount = m_pNewInventoryCtrl->GetNumberOfColumn();
+    const int rowCount = m_pNewInventoryCtrl->GetNumberOfRow();
+    std::vector<ArrangeItem> items;
+
+    const size_t itemCount = m_pNewInventoryCtrl->GetNumberOfItems();
+    items.reserve(itemCount);
+    for (size_t i = 0; i < itemCount; ++i)
+    {
+        ITEM* pItem = m_pNewInventoryCtrl->GetItem(static_cast<int>(i));
+        if (pItem == nullptr || pItem->Type == -1)
+        {
+            continue;
+        }
+
+        const ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[pItem->Type];
+        if (!IsAllowedArrangeItemSize(pItemAttr->Width, pItemAttr->Height))
+        {
+            return false;
+        }
+
+        const int sourceIndex = m_pNewInventoryCtrl->GetIndexByItem(pItem);
+        if (sourceIndex < MAX_EQUIPMENT)
+        {
+            return false;
+        }
+
+        items.push_back({
+            pItem->Key,
+            sourceIndex,
+            pItem->x,
+            pItem->y,
+            pItem->x,
+            pItem->y,
+            pItemAttr->Width,
+            pItemAttr->Height
+        });
+    }
+
+    if (!ComputeArrangeTargets(items, columnCount, rowCount))
+    {
+        return false;
+    }
+
+    std::vector<InventoryRearrangeMove> plannedMoves;
+    if (!GenerateArrangeMoves(items, columnCount, rowCount, plannedMoves))
+    {
+        return false;
+    }
+    if (plannedMoves.empty())
+    {
+        return false;
+    }
+
+    moves = plannedMoves;
+    return true;
+}
+
+void CNewUIMyInventory::ProcessInventoryRearrange()
+{
+    if (!m_bInventoryRearrangePending)
+    {
+        return;
+    }
+    if (EquipmentItem)
+    {
+        return;
+    }
+    if (!CanUseInventoryRearrange())
+    {
+        m_bInventoryRearrangePending = false;
+        return;
+    }
+    if (m_bInventoryRearrangeMoveInFlight)
+    {
+        if (m_InventoryRearrangeMoves.empty())
+        {
+            m_bInventoryRearrangeMoveInFlight = false;
+            m_bInventoryRearrangePending = false;
+            return;
+        }
+
+        const InventoryRearrangeMove& move = m_InventoryRearrangeMoves.front();
+        const DWORD oldItemKey = move.itemKey;
+        ITEM* pItem = m_pNewInventoryCtrl->FindItem(move.targetIndex);
+        if (pItem == nullptr || m_pNewInventoryCtrl->GetIndexByItem(pItem) != move.targetIndex)
+        {
+            m_bInventoryRearrangeMoveInFlight = false;
+            m_bInventoryRearrangePending = false;
+            m_InventoryRearrangeMoves.clear();
+            return;
+        }
+
+        ReplaceArrangeMoveItemKey(m_InventoryRearrangeMoves, oldItemKey, pItem->Key);
+        m_InventoryRearrangeMoves.erase(m_InventoryRearrangeMoves.begin());
+        m_bInventoryRearrangeMoveInFlight = false;
+    }
+
+    if (m_InventoryRearrangeMoves.empty())
+    {
+        m_bInventoryRearrangePending = false;
+        return;
+    }
+
+    if (!ExecuteNextInventoryRearrangeMove())
+    {
+        m_bInventoryRearrangePending = false;
+        m_InventoryRearrangeMoves.clear();
+    }
+}
+
+void CNewUIMyInventory::RequestInventoryRearrange()
+{
+    if (!CanUseInventoryRearrange())
+    {
+        return;
+    }
+
+    if (!BuildInventoryRearrangeMoves(m_InventoryRearrangeMoves))
+    {
+        m_InventoryRearrangeMoves.clear();
+        return;
+    }
+
+    m_bInventoryRearrangePending = true;
+    m_bInventoryRearrangeMoveInFlight = false;
+    ProcessInventoryRearrange();
+}
+
+bool CNewUIMyInventory::ExecuteNextInventoryRearrangeMove()
+{
+    if (m_pNewInventoryCtrl == nullptr || m_InventoryRearrangeMoves.empty())
+    {
+        return false;
+    }
+
+    const InventoryRearrangeMove& move = m_InventoryRearrangeMoves.front();
+    ITEM* pItem = m_pNewInventoryCtrl->FindItemByKey(move.itemKey);
+    if (pItem == nullptr)
+    {
+        return false;
+    }
+
+    const int sourceIndex = m_pNewInventoryCtrl->GetIndexByItem(pItem);
+    if (sourceIndex == move.targetIndex)
+    {
+        m_InventoryRearrangeMoves.erase(m_InventoryRearrangeMoves.begin());
+        return true;
+    }
+    if (!CNewUIInventoryCtrl::CreatePickedItem(m_pNewInventoryCtrl, pItem))
+    {
+        return false;
+    }
+
+    CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();
+    if (pPickedItem == nullptr || pPickedItem->GetItem() == nullptr)
+    {
+        CNewUIInventoryCtrl::BackupPickedItem();
+        return false;
+    }
+
+    m_pNewInventoryCtrl->RemoveItem(pItem);
+    pPickedItem->HidePickedItem();
+
+    if (!SendRequestEquipmentItem(STORAGE_TYPE::INVENTORY, sourceIndex,
+        pPickedItem->GetItem(), STORAGE_TYPE::INVENTORY, move.targetIndex))
+    {
+        CNewUIInventoryCtrl::BackupPickedItem();
+        return false;
+    }
+
+    m_bInventoryRearrangeMoveInFlight = true;
+    return true;
+}
```

---

## 16. `src/source/UI/NewUI/Inventory/NewUIMyInventory.h`
* **Change Summary:** Added rearrange variables, `CNewUIButton m_BtnArrange`, `InventoryRearrangeMove` struct, and action member functions to the inventory class declaration.
* **Line-by-Line Diff:**
```diff
@@ -14,10 +14,17 @@
 #include "UI/NewUI/Inventory/NewUIInventoryActionController.h"
 #include "GameLogic/Items/IInventoryActionContext.h"
 #include <span>
 #include <vector>
 #include "Core/Globals/_enum.h"
 
 namespace SEASON3B
 {
+    struct InventoryRearrangeMove
+    {
+        DWORD itemKey;
+        int targetIndex;
+    };
+
     class CNewUIMyInventory
         : public CNewUIObj
         , public INewUI3DRenderObj
@@ -87,6 +94,7 @@ namespace SEASON3B
         CNewUIButton m_BtnExit;
         CNewUIButton m_BtnMyShop;
         CNewUIButton m_BtnExpand;
+        CNewUIButton m_BtnArrange;
 
         MYSHOP_MODE m_MyShopMode;
         SEASON3B::REPAIR_MODE m_RepairMode;
@@ -94,6 +102,9 @@ namespace SEASON3B
 
         bool m_bRepairEnableLevel;
         bool m_bMyShopOpen;
+        bool m_bInventoryRearrangePending;
+        bool m_bInventoryRearrangeMoveInFlight;
+        std::vector<InventoryRearrangeMove> m_InventoryRearrangeMoves;
 
     public:
         CNewUIMyInventory();
@@ -197,6 +208,11 @@ namespace SEASON3B
         bool EquipmentWindowProcess();
         bool InventoryProcess() const;
         bool BtnProcess();
+        bool BuildInventoryRearrangeMoves(std::vector<InventoryRearrangeMove>& moves) const;
+        bool CanUseInventoryRearrange() const;
+        bool ExecuteNextInventoryRearrangeMove();
+        void ProcessInventoryRearrange();
+        void RequestInventoryRearrange();
 
         void RenderItemToolTip(int iSlotIndex) const;
         bool CanOpenMyShopInterface();
```

---

## 17. `src/source/UI/NewUI/NewUIMuHelper.cpp`
* **Change Summary:**
  1. Configured Mu Helper custom setup to allow active Summoner skills (Berserker, Lightning Orb, Blind, Drainlife) to be added to active combat slots even when classified as master-tier inside upstream game configurations.
  2. Removed Summoner active draining attacks from classification under `IsHealingSkill` to align with active combat rules.
* **Line-by-Line Diff:**
```diff
@@ -1913,7 +1913,23 @@ void CNewUIMuHelperSkillList::PrepareSkillsToRender()
             {
                 BYTE bySkillUseType = SkillAttribute[iSkillType].SkillUseType;
 
-                if (bySkillUseType == SKILL_USE_TYPE_MASTER || bySkillUseType == SKILL_USE_TYPE_MASTERLEVEL)
+                // Allow known Summoner active skills through even when flagged as master-type,
+                // because the game data marks them as master skills but they are actively usable.
+                const bool isSummonerActiveSkill =
+                    iSkillType == AT_SKILL_ALICE_DRAINLIFE      ||
+                    iSkillType == AT_SKILL_ALICE_DRAINLIFE_STR  ||
+                    iSkillType == AT_SKILL_ALICE_CHAINLIGHTNING ||
+                    iSkillType == AT_SKILL_ALICE_CHAINLIGHTNING_STR ||
+                    iSkillType == AT_SKILL_ALICE_LIGHTNINGORB   ||
+                    iSkillType == AT_SKILL_ALICE_BLIND          ||
+                    iSkillType == AT_SKILL_ALICE_WEAKNESS       ||
+                    iSkillType == AT_SKILL_ALICE_ENERVATION     ||
+                    iSkillType == AT_SKILL_ALICE_BERSERKER      ||
+                    iSkillType == AT_SKILL_ALICE_BERSERKER_STR  ||
+                    iSkillType == AT_SKILL_ALICE_THORNS;
+
+                if (!isSummonerActiveSkill &&
+                    (bySkillUseType == SKILL_USE_TYPE_MASTER || bySkillUseType == SKILL_USE_TYPE_MASTERLEVEL))
                 {
                     continue;
                 }
@@ -2244,9 +2260,6 @@ bool CNewUIMuHelperSkillList::IsHealingSkill(int iSkillType)
     case AT_SKILL_HEALING:
     case AT_SKILL_HEALING_STR:
         return true;
-    case AT_SKILL_ALICE_DRAINLIFE:
-    case AT_SKILL_ALICE_DRAINLIFE_STR:
-        return true;
     }
 
     return false;
```
