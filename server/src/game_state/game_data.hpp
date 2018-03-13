#pragma once
namespace game_data {
    const char* const SATELLITE_NAME_POOL[] = {"Moon", "Phobos", "Deimos", "Io", "Europa", "Ganymede", "Callisto", "Amalthea" };
    const char* const PLANET_NAME_POOL[] = {"Mercury", "Venus", "Earth", "Moon", "Mars", "Jupiter", "Io", "Europa" };
    const char* const STAR_NAME_POOL[] = {"Acamar", "Achernar"};
    const char* const GALAXY_NAME_POOL[] = {"Andromeda", "Black Eye"};
    const char* const UNIVERSE_NAME_POOL[] = {"Cosmos", "Chaos", "World", "Nature"};

    const int SATELLITE_NAME_POOL_SIZE = sizeof(SATELLITE_NAME_POOL) / sizeof(SATELLITE_NAME_POOL[0]);
    const int PLANET_NAME_POOL_SIZE = sizeof(PLANET_NAME_POOL) / sizeof(PLANET_NAME_POOL[0]);
    const int STAR_NAME_POOL_SIZE = sizeof(STAR_NAME_POOL) / sizeof(STAR_NAME_POOL[0]);
    const int GALAXY_NAME_POOL_SIZE = sizeof(GALAXY_NAME_POOL) / sizeof(GALAXY_NAME_POOL[0]);
    const int UNIVERSE_NAME_POOL_SIZE = sizeof(UNIVERSE_NAME_POOL) / sizeof(UNIVERSE_NAME_POOL[0]);

    const float EARTH_MASS = 6e24;
};
