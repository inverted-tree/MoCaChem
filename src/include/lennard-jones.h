#pragma once

#include "config.h"
#include "data.h"

//******************************************************************************
// Interface Dataype Definitions
//******************************************************************************

typedef struct {
	double lennard_jones;
	double virial;
} mcc_Energy_t;

//******************************************************************************
// Interface Function Declarations
//******************************************************************************

mcc_Energy_t mcc_lennard_jones_particle_potential(int index,
                                                  mcc_Particle_t *particle,
                                                  mcc_Config_t *config);

mcc_Energy_t mcc_lennard_jones_system_potential(mcc_Config_t *config);
