///////////////////////////////////////////////////////////////////////////////
//            Copyright (C) 2004-2011 by The Allacrost Project
//            Copyright (C) 2012-2016 by Bertram (Valyria Tear)
//                         All Rights Reserved
//
// This code is licensed under the GNU GPL version 2. It is free software
// and you may modify it and/or redistribute it under the terms of this license.
// See http://www.gnu.org/copyleft/gpl.html for details.
///////////////////////////////////////////////////////////////////////////////

/** ****************************************************************************
*** \file    map_zones.h
*** \author  Guillaume Anctil, drakkoon@allacrost.org
*** \author  Yohann Ferreira, yohann ferreira orange fr
*** \brief   Header file for map mode zones.
*** ***************************************************************************/

#ifndef __MAP_ZONES_HEADER__
#define __MAP_ZONES_HEADER__

#include "modes/map/map_mode.h"
#include "modes/map/map_objects.h"

namespace vt_map
{

namespace private_map
{

class EnemySprite;

/** ****************************************************************************
*** \brief Represents a rectangular area on a map.
***
*** The area is represented by the coordinates of the top-left and bottom-right
*** corners. Both are represented in the row / column format of collision grid
*** elements. Zone sections can only include entire grid elements, not portions
*** of an element.
***
*** \note The primary intent of this class is to be able to combine several
*** ZoneSections to create a non-rectangular shape. This is how a map zone is
*** formed.
*** ***************************************************************************/
class ZoneSection
{
public:
    ZoneSection(uint16_t left, uint16_t right, uint16_t top, uint16_t bottom) :
        left_col(left), right_col(right), top_row(top), bottom_row(bottom)
    {
    }

    //! \brief Collision grid columns for the top and bottom section of the area
    uint16_t left_col, right_col;

    //! \brief Collision grid rows for the top and bottom section of the area
    uint16_t top_row, bottom_row;
};

/** ****************************************************************************
*** \brief Represents a zone on a map that can take any shape
***
*** The area is made up of many ZoneSection instances, so it can be any shape
*** (specifically, any combination of rectangular shapes).
*** A MapZone by itself is not very useful, but serves as
*** a foundation for other zone classes which derive from it.
***
*** \note ZoneSections in the MapZone may overlap without any problem. In general,
*** however, you should try to create a MapZone using as few ZoneSections as possible
*** to improve performance.
*** ***************************************************************************/
class MapZone
{
    // This friend declaration is necessary because EnemyZone, although it derives from MapZone, also keeps a pointer
    // to a MapZone object and needs to access the protected members and methods of this object pointer.
    friend class EnemyZone;

public:
    /** \brief Constructs a map zone that is initialized with a single zone section
    *** \param left_col The left edge of the section to add
    *** \param right_col The right edge of the section to add
    *** \param top_row The top edge of the section to add
    *** \param bottom_row The bottom edge of the section to add
    **/
    MapZone(uint16_t left_col, uint16_t right_col, uint16_t top_row, uint16_t bottom_row);

    virtual ~MapZone();

    //! \brief A C++ wrapper made to create a new object from scripting,
    //! without letting Lua handling the object life-cycle.
    //! \note We don't permit luabind to use constructors here as it can't currently
    //! give the object ownership at construction time.
    static MapZone* Create(uint16_t left_col, uint16_t right_col, uint16_t top_row, uint16_t bottom_row);

    /** \brief Adds a new zone section to the map zone
    *** \param left_col The left edge of the section to add
    *** \param right_col The right edge of the section to add
    *** \param top_row The top edge of the section to add
    *** \param bottom_row The bottom edge of the section to add
    ***
    *** \note The value of left shoult be less than right, and top should be less that
    *** bottom. If these conditions are not true, a warning will be printed and the section
    *** will not be added.
    **/
    virtual void AddSection(uint16_t left_col, uint16_t right_col, uint16_t top_row, uint16_t bottom_row);

    //! \brief Updates the state of the zone
    virtual void Update();

    /** \brief Returns true if the position coordinates are located inside the zone (inclusive to the zone boundary edges)
    *** \param pos_x The x position to check
    *** \param pos_y The y position to check
    ***
    *** \note This function ignores the fractional part of map coordinates for performance reasons. So whenever an object is being
    *** checked as to whether or not it may be found in this zone, the floating point portion of its map coordinates are not taken
    *** into account.
    **/
    bool IsInsideZone(float pos_x, float pos_y) const;

    //! \brief Draws the map zone on screen for debugging purpose
    virtual void Draw();

    /** \brief Returns random x, y position coordinates within the zone
    *** \param x A reference where to store the value of the x position
    *** \param y A reference where to store the value of the y position
    **/
    void RandomPosition(float &x, float &y);

    //! \brief Loads the current animation file as the new interaction icon of the object.
    void SetInteractionIcon(const std::string& animation_filename);

    //! \brief Draws the interaction icon at the top of the zone rectangle, if any.
    void DrawInteractionIcon();

protected:
    //! \brief The rectangular sections which compose the map zone
    std::vector<ZoneSection> _sections;

    //! \brief Interaction icon
    vt_video::AnimatedImage* _interaction_icon;

    //! \brief Tells whether a section is on screen and place the drawing cursor in that case.
    bool _ShouldDraw(const ZoneSection &section);

private:
    //
    // The copy constructor and assignment operator are hidden by design
    // to cause compilation errors when attempting to copy or assign this class.
    //

    MapZone(const MapZone& map_zone);
    MapZone& operator=(const MapZone& map_zone);
};

/** ****************************************************************************
*** \brief A zone which tracks when the map camera enters or exits
***
*** A typical use of map zones is to track when the sprite controlled by the player
*** (usually pointed to by the map camera) enters or exits a zone, triggering a
*** map event. This class makes that common case a little easier to implement in
*** map scripting code.
***
*** \note An important issue to remember is that the map camera may be changed to point
*** at any sprite at any given time. This class is not informed of such events, therefore
*** a sprite may be seen as "entering" the zone
***
*** \note This zone is less powerful than the ResidentZone class, which tracks all
*** sprites status relative to the zone. CameraZone is much simpler than ResidentZone
*** in terms of computational costs (for both processor and memory requirements) and
*** thus should be utilized when the additional features of ResidentZone are not required.
*** ***************************************************************************/
class CameraZone : public MapZone
{
public:
    /** \brief Constructs a camera zone that is initialized with a single zone section
    *** \param left_col The left edge of the section to add
    *** \param right_col The right edge of the section to add
    *** \param top_row The top edge of the section to add
    *** \param bottom_row The bottom edge of the section to add
    **/
    CameraZone(uint16_t left_col, uint16_t right_col, uint16_t top_row, uint16_t bottom_row);

    virtual ~CameraZone() override
    {
    }

    //! \brief A C++ wrapper made to create a new object from scripting,
    //! without letting Lua handling the object life-cycle.
    //! \note We don't permit luabind to use constructors here as it can't currently
    //! give the object ownership at construction time.
    static CameraZone* Create(uint16_t left_col, uint16_t right_col, uint16_t top_row, uint16_t bottom_row);

    //! \brief Updates the state of the zone by checking the current camera position
    virtual void Update() override;

    //! \brief Returns true if the sprite pointed to by the camera is located within the zone
    bool IsCameraInside() const {
        return _camera_inside;
    }

    //! \brief Returns true if the sprite pointed to by the camera is entering the zone
    bool IsCameraEntering() const {
        return ((_camera_inside == true) && (_was_camera_inside == false));
    }

    //! \brief Returns true if the sprite pointed to by the camera is leaving the zone
    bool IsCameraExiting() const {
        return ((_camera_inside == false) && (_was_camera_inside == true));
    }

protected:
    //! \brief Set to true when the sprite pointed to by the camera is inside this zone
    bool _camera_inside;

    //! \brief Holds the previous value of _camera_inside
    bool _was_camera_inside;

private:
    //
    // The copy constructor and assignment operator are hidden by design
    // to cause compilation errors when attempting to copy or assign this class.
    //

    CameraZone(const CameraZone& camera_zone);
    CameraZone& operator=(const CameraZone& camera_zone);
};


/** ****************************************************************************
*** \brief Represents an area where enemy sprites spawn and roam in.
***
*** This zone will spawn enemy sprites somewhere within its boundaries. It also
*** regenerates dead enemies after a certain amount of time. The enemies can be
*** constrained within the zone area or be made free to roam the entire map
*** after spawning. If desired, the class has the option to declare seperate zones
*** for where enemies may spawn and where they may roam.
***
*** \note It makes no sense to use seperate zones for spawning if the enemies are
*** not restrained in their roaming. If free roaming is enabled for this zone, construct
*** the zone sections the standard way.
***
*** \note By default, enemies are restricted to move about within their zones and
*** a default regeneration timer is used. These properties can be changed after
*** the class object is constructed.
***
*** \todo Consider adding a call that will disable additional enemies from spawning in
*** the zone, or limit the maximum number of enemies that may ever spawn in the zone.
*** ***************************************************************************/
class EnemyZone : public MapZone
{
public:
    /** \brief Constructs an enemy zone that is initialized with a single zone section
    *** \param left_col The left edge of the section to add
    *** \param right_col The right edge of the section to add
    *** \param top_row The top edge of the section to add
    *** \param bottom_row The bottom edge of the section to add
    **/
    EnemyZone(uint16_t left_col, uint16_t right_col,
              uint16_t top_row, uint16_t bottom_row);

    virtual ~EnemyZone() override;

    //! \brief A C++ wrapper made to create a new object from scripting,
    //! without letting Lua handling the object life-cycle.
    //! \note We don't permit luabind to use constructors here as it can't currently
    //! give the object ownership at construction time.
    static EnemyZone* Create(uint16_t left_col, uint16_t right_col, uint16_t top_row, uint16_t bottom_row);

    //! \brief Enables/disables the enemy zone.
    void SetEnabled(bool is_enabled) {
        _enabled = is_enabled;
    }

    /** \brief Adds a new enemy sprite to the zone
    *** \param enemy A pointer to the EnemySprite object instance to add
    *** \param count The number of copies of this enemy to add
    **/
    void AddEnemy(EnemySprite *enemy, uint8_t count = 1);

    /** \brief Adds a new zone section to the zone where enemies may spawn
    *** \param left_col The left edge of the section to add
    *** \param right_col The right edge of the section to add
    *** \param top_row The top edge of the section to add
    *** \param bottom_row The bottom edge of the section to add
    ***
    *** Calling this method automatically enables this zone to have seperate areas for spawning
    *** and roaming. Upon successfully adding a single spawn section, enemies will no longer be
    *** allowed to spawn in the sections added directly to the class object. Each spawn zone section
    *** must reside completely inside a roaming zone section for it to be added successfully. The reason
    *** for this is that we do not want to allow enemies to spawn at a location where they are "stuck" and
    *** can't roam anywhere. And for zones that do not restrict roaming, it makes no sense to add zones
    *** specific to spawning in the first place.
    ***
    *** \note The value of left shoult be less than right, and top should be less that
    *** bottom. If these conditions are not true, a warning will be printed and the section
    *** will not be added.
    **/
    void AddSpawnSection(uint16_t left_col, uint16_t right_col, uint16_t top_row, uint16_t bottom_row);

    //! \brief Decrements the number of active enemies by one
    void EnemyDead();

    //! \brief Gradually spawns enemy sprites in the zone
    virtual void Update() override;

    //! \brief Draw the zone on screen for debugging purpose
    virtual void Draw() override;

    //! \brief Returns true if this zone has seperate zones for roaming and spawning
    bool HasSeparateSpawnZone() const {
        return (_spawn_zone != nullptr);
    }

    //! \name Class Member Access Functions
    //@{
    bool IsRoamingRestrained() const {
        return _roaming_restrained;
    }

    uint32_t GetSpawnTime() const {
        return _spawn_timer.GetDuration();
    }

    void SetRoamingRestrained(bool restrain) {
        _roaming_restrained = restrain;
    }

    //! \note Calling this function will reset the elapsed spawn time
    void SetSpawnTime(uint32_t time) {
        _spawn_timer.Reset();
        _spawn_timer.SetDuration(time);
        _spawn_timer.Run();
    }

    //! Set the number of times an enemy can spawn in this enemy zone.
    void SetSpawnsLeft(int32_t spawns)
    { _spawns_left = spawns; }

    //! Set the number of times left an enemy can spawn in this enemy zone.
    int32_t GetSpawnsLeft() const
    { return _spawns_left; }

    //! Decrease enemy spawns left to do. Should be triggered upon enemy death.
    void DecreaseSpawnsLeft()
    { if (_spawns_left > 0) --_spawns_left; }
    //@}

private:
    //
    // The copy constructor and assignment operator are hidden by design
    // to cause compilation errors when attempting to copy or assign this class.
    //

    EnemyZone(const EnemyZone& enemy_zone);
    EnemyZone& operator=(const EnemyZone& enemy_zone);

    //! \brief Tells whether the zone is activated.
    bool _enabled;

    //! \brief If true, enemies of this zone are not allowed
    //! to roam outside of the zone boundaries
    bool _roaming_restrained;

    //! \brief The number of enemies that are currently not in the DEAD state
    uint8_t _active_enemies;

    //! \brief The number of times an enemy can (re)spawn in this enemy zone.
    //! By default, this value is equal to -1 meaning an infinite amount of time.
    //! This permits to set special spawn points where enemies can only be seen once,
    //! for special bosses or puzzles.
    int32_t _spawns_left;

    //! \brief Used for the respawning of enemies within the zone
    vt_system::SystemTimer _spawn_timer;

    //! \brief Used to keep track of the amount of time to wait before re-spawning a dead enemy
    vt_system::SystemTimer _dead_timer;

    //! \brief An optional zone which specifies where enemies may spawn
    MapZone *_spawn_zone;

    /** \brief Contains all of the enemies that may exist in this zone.
    *** \note These sprites will be deleted by the map object manager, not the destructor of this class.
    **/
    std::vector<EnemySprite*> _enemies;

    /** \brief Contains all of the enemies that are owned by this class.  These objects must be cleaned up by this class.
    ***        This is a hack around a memory leak and should be addressed more formally in the future.
    **/
    std::vector<EnemySprite*> _enemies_owned;
};

} // namespace private_map

} // namespace vt_map

#endif // __MAP_ZONES_HEADER__
