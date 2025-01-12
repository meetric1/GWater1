AddCSLuaFile()

SWEP.Base = "weapon_base"
SWEP.PrintName = "GWater pickup Gun"

SWEP.ViewModel = "models/weapons/c_pistol.mdl"
SWEP.ViewModelFlip = false
SWEP.UseHands = true

SWEP.WorldModel = "models/weapons/w_pistol.mdl"
SWEP.SetHoldType = "pistol"

SWEP.Weight = 5
SWEP.AutoSwitchTo = true
SWEP.AutoSwitchFrom = false

SWEP.Category = "GWater"
SWEP.Slot = 0
SWEP.SlotPos = 1

SWEP.DrawAmmo = true
SWEP.DrawCrosshair = true

SWEP.Spawnable = true
SWEP.AdminSpawnable = false

SWEP.Primary.ClipSize = -1
SWEP.Primary.DefaultClip = -1
SWEP.Primary.Ammo = "none"
SWEP.Primary.Automatic = false

SWEP.Secondary.ClipSize = -1
SWEP.Secondary.DefaultClip = -1
SWEP.Secondary.Ammo = "none"
SWEP.Secondary.Automatic = false

if SERVER then return end

local dist = -1
local particleIndex = -1
local initialMass
local function handlePhysgun(ply)
	-- handle physgun
	if ply:IsValid() and ply:GetActiveWeapon():IsValid() and ply:GetActiveWeapon():GetClass() == "weapon_gwater_pickup" then
		local mouseHeld = ply:KeyDown(IN_ATTACK2)
		local fwd = ply:EyePos() + ply:EyeAngles():Forward() * dist

		if dist == -1 and mouseHeld then
			local newDist, pos, index = gwater.TraceLine(ply:EyePos(), ply:EyeAngles():Forward())
			if newDist > 0 then 
				initialMass = gwater.SetParticlePos(index, fwd, 1 / 50000)
				particleIndex = index
				dist = newDist
			end
		end
		
		if dist > 0 then
			if mouseHeld then
				gwater.SetParticlePos(particleIndex, fwd, 1 / 50000)
			else
				gwater.SetParticlePos(particleIndex, fwd, initialMass)
				dist = -1
			end
		end
	end
end

local function handleGravgun(ply)
	if ply:IsValid() and ply:GetActiveWeapon():IsValid() and ply:GetActiveWeapon():GetClass() == "weapon_gwater_pickup" and ply:KeyDown(IN_ATTACK) then
		gwater.ApplyForceOutwards(ply:EyePos() + ply:EyeAngles():Forward() * 125 + Vector(0, 0, 10), -3, 100, false);
	end
end


timer.Create("Gwater_Pickup", 0, 0.1, function()
	for k, v in ipairs(ents.FindByClass("player")) do
		handleGravgun(v)
		handlePhysgun(v)
	end
end)
