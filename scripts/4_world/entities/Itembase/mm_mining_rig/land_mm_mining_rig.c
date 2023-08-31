class land_mm_mining_rig : Inventory_Base
{
    private const string SOUND_MiningRig_START = "MiningRig_START_SoundSet";
    private const string SOUND_MiningRig_STOP = "MiningRig_STOP_SoundSet";
    private const string SOUND_MiningRig_MINED = "MiningRig_MINED_SoundSet";

    private EffectSound m_StartSound;
    private EffectSound m_StopSound;
    private EffectSound m_MinedSound;

    private bool m_IsMining = false;
    private bool m_IsDamaged = false;
    private float m_SuccessChance = 0.5;
    private int m_DamageChance = 50;
    private float m_DamagePercent = 0.1;
    private float m_MineInterval = 5.0;

    private ref Timer m_MiningTimer;

    private ref map<string, int> m_SlotNameToIdMap;

    void land_mm_mining_rig()
    {
        RegisterNetSyncVariableBool("m_IsSoundSynchRemote");
        m_MiningTimer = new Timer(CALL_CATEGORY_SYSTEM);
        
        m_SlotNameToIdMap = new map<string, int>;
        InitSlotNameToIdMap();
    }

   void AttachGPU(ItemBase gpuItem)
{
    if (gpuItem && gpuItem.IsInherited(ItemBase) && !GetGame().IsMultiplayer())
    {
        int emptySlotIndex = -1;
        for (int i = 0; i < 10; i++)
        {
            string attachmentSlot = "Gpu" + (i + 1);
            EntityAI gpuAttachment = FindAttachmentBySlotName(attachmentSlot);

            if (!gpuAttachment)
            {
                emptySlotIndex = i;
                break;
            }
        }

        if (emptySlotIndex >= 0)
        {
            string availableAttachmentSlot = "Gpu" + (emptySlotIndex + 1);
            TakeEntityAsAttachmentEx(gpuItem, availableAttachmentSlot);
            Print("Attached GPU to slot: " + availableAttachmentSlot);
        }
        else
        {
            Print("No empty GPU slots available.");
        }
    }
    else
    {
        Print("Invalid GPU item or multiplayer mode.");
    }
}

void InitSlotNameToIdMap()
{
    m_SlotNameToIdMap.Insert("Gpu", InventorySlots.Gpu);
    m_SlotNameToIdMap.Insert("Gpu", InventorySlots.Gpu);
    m_SlotNameToIdMap.Insert("Gpu", InventorySlots.Gpu);
    m_SlotNameToIdMap.Insert("Gpu", InventorySlots.Gpu);
    m_SlotNameToIdMap.Insert("Gpu", InventorySlots.Gpu);
    m_SlotNameToIdMap.Insert("Gpu", InventorySlots.Gpu);
    m_SlotNameToIdMap.Insert("Gpu", InventorySlots.Gpu);
    m_SlotNameToIdMap.Insert("Gpu", InventorySlots.Gpu);
    m_SlotNameToIdMap.Insert("Gpu", InventorySlots.Gpu);
    m_SlotNameToIdMap.Insert("Gpu", InventorySlots.Gpu);
    
    m_SlotNameToIdMap.Insert("bitcoin", InventorySlots.bitcoin);
    
    for (int i = 1; i <= 10; i++)
    {
        m_SlotNameToIdMap.Insert("Gpu" + i, InventorySlots.Gpu + i - 1);
    }
    m_SlotNameToIdMap.Insert("bitcoin", InventorySlots.bitcoin);
}
    
    void TakeEntityAsAttachmentEx(EntityAI entity, string slotName)
    {
        if (entity && GetInventory())
        {
            int slotId = GetSlotIdFromName(slotName);
            
            if (slotId != -1)
            {
                InventoryLocation loc = new InventoryLocation;
                if (GetInventory().FindFreeLocationFor(entity, FindInventoryLocationType.ANY, loc))
                {
                    if (loc.IsValid())
                    {
                        int attachmentSlotId = InventorySlots.GetSlotIdFromString(slotName); 
                        loc.Copy(entity.GetInventory().GetCurrentInventoryLocation());
                        loc.SetParent(this); 
                        loc.SetAttachment(attachmentSlotId); 
                        if (!GetInventory().LocationSyncMoveEntity(loc, entity))
                        {
                            Error("land_mm_mining_rig: Failed to attach entity to slot: " + slotName);
                        }
                    }
                    else
                    {
                        Error("land_mm_mining_rig: Failed to get valid location for attachment.");
                    }
                }
                else
                {
                    Error("land_mm_mining_rig: No free location for attachment.");
                }
            }
            else
            {
                Error("land_mm_mining_rig: Invalid slot name: " + slotName);
            }
        }
    }

    

    private void DamageGPU()
    {
        for (int i = 1; i <= 10; i++)
        {
            string attachmentSlot = "Gpu" + i;
            ItemBase gpuItem = ItemBase.Cast(FindAttachmentBySlotname(attachmentSlot));

            if (gpuItem && !gpuItem.IsRuined())
            {
                gpuItem.DecreaseHealth("", "", gpuItem.GetMaxHealth("", "") * m_DamagePercent);
                m_IsDamaged = true;
                return;
            }
        }
    }

    bool CanStartMining()
    {
        if (!m_PowerGenerator || !m_CableReel)
        {
            return false;
        }

        for (int i = 1; i <= 10; i++)
        {
            string attachmentSlot = "Gpu" + i;
            EntityAI gpuItem = FindAttachmentBySlotname(attachmentSlot);

            if (gpuItem)
            {
                return true;
            }
        }

        return false;
    }

    void IncrementBitcoin()
    {
        if (GetGame().IsServer()) 
        {
            ItemBase bitcoinItem = ItemBase.Cast(GetGame().CreateObject("bitcoin", GetPosition())); 
            if (bitcoinItem)
            {
                GetInventory().TakeEntityAsAttachmentEx(bitcoinItem, "bitcoin");  
            }
        }
    }    

    private void PlayStartSound()
    {
        PlaySoundSet(m_StartSound, SOUND_MiningRig_START, 0.1, 0.1);
    }

    private void PlayStopSound()
    {
        PlaySoundSet(m_StopSound, SOUND_MiningRig_STOP, 0.1, 0.1);
    }

    private void PlayMinedSound()
    {
        PlaySoundSet(m_MinedSound, SOUND_MiningRig_MINED, 0.1, 0.1);
    }

    override void OnWorkStart()
    {
        PlayStartSound();
        StartMining();
    }

    override void OnWork(float consumed_energy)
    {
        super.OnWork(consumed_energy);

        if (m_IsMining && m_MiningTimer)
        {
            m_MiningTimer.Run(m_MineInterval, this, "Mine", null, true);
            IncrementBitcoin();  
        }
    }

    override void OnWorkStop()
    {
        PlayStopSound();
        StopMining();
    }

    private void StartMining()
    {
        if (!m_IsMining)
        {
            m_IsMining = true;
            m_MiningTimer.Run(m_MineInterval, this, "Mine", null, true);
        }
    }

    private void StopMining()
    {
        if (m_IsMining)
        {
            m_IsMining = false;
            m_MiningTimer.Stop();
        }
    }

    private void Mine()
    {
        if (GetCompEM().IsWorking() && m_SuccessChance > Math.RandomFloat(0, 1))
        {
            IncrementBitcoin();
            PlayMinedSound();

            if (Math.RandomInt(0, 100) <= m_DamageChance && !m_IsDamaged)
            {
                DamageGPU();
                m_IsDamaged = true;
            }
        }
    }

    bool CanMine()
    {
        if (!GetCompEM().CanWork())
        {
            return false;
        }
        return true;
    }
}
