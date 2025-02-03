/*
 * #include "data-octree.h"
#include "config.h"
#include "data.h"
#include "include/utils.h"
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

//******************************************************************************
//  Data Representation Definition
//******************************************************************************

struct mcc_Index_t {
    size_t bin_id;
    size_t particle_id;
    size_t array_id;
};

struct mcc_Octree_Particle_t {
    bool is_valid;
    mcc_Particle_t *value;
};

struct mcc_Data_Octree_t {
    bool is_initialized;

    size_t depth;
    size_t particle_count;
    size_t fill_count;
    size_t bin_count;
    size_t bins_per_dim;
    size_t bin_size;
    double bin_width;

    struct mcc_Octree_Particle_t **bins;
    mcc_Particle_t *particles;
};

size_t const BINS_TO_COMP = 27;
struct mcc_Particle_Iterator_State_t {
    mcc_Index_t index;
    size_t currb;
    size_t currp;
    struct mcc_Octree_Particle_t *bins[BINS_TO_COMP];
};

struct mcc_Data_Octree_t octree_data = {
    .is_initialized = false,
    .depth = 0,
    .particle_count = 0,
    .fill_count = 0,
    .bin_count = 0,
    .bins_per_dim = 0,
    .bin_size = 0,
    .bin_width = 0.0,
    .bins = NULL,
    .particles = NULL,
};

struct mcc_Particle_Iterator_State_t octree_iter = {
    .index = {0},
    .currb = 0,
    .currp = 0,
    .bins = {NULL},
};

//******************************************************************************
//  Internal function declarations
//******************************************************************************

static inline size_t mcc_data_octree_calc_bin(mcc_Particle_t *p);

mcc_Status_t mcc_data_octree_initalize(mcc_Config_t *config);

mcc_Status_t mcc_data_octree_finalize();

mcc_Particle_t *mcc_data_octree_get_particle(mcc_Index_t *index);

bool mcc_data_octree_set_particle(mcc_Index_t *index, mcc_Particle_t *particle);

mcc_Particle_t *mcc_data_octree_iterator_next();

bool mcc_data_octree_iterator_is_done(mcc_Particle_t *result);

void mcc_data_octree_check_initialized();

void mcc_data_octree_check_index(mcc_Index_t *index);

static inline size_t mcc_utils_octree_wraparound(int i);

//******************************************************************************
//  Interface function definitions
//******************************************************************************

mcc_DAF_t mcc_data_octree_get_access_functions() {
    mcc_DAF_t fs = {
        .init = mcc_data_octree_initalize,
        .destroy = mcc_data_octree_finalize,
        .get_particle = mcc_data_octree_get_particle,
        .set_particle = mcc_data_octree_set_particle,
    };
    return fs;
}

mcc_Particle_Iterator_t mcc_data_octree_get_iterator(mcc_Index_t *index,
                                                     double cutoff) {
    (void)cutoff;
    octree_iter.index = *index;
    octree_iter.currb = 0;
    octree_iter.currp = 0;

    for (size_t i = 0; i < 27; i++) {
        size_t bin_x = mcc_utils_octree_wraparound(
            index->bin_id % octree_data.bins_per_dim + (i % 3 - 1));
        size_t bin_y = mcc_utils_octree_wraparound(
            (int)(index->bin_id / octree_data.bins_per_dim) %
                octree_data.bins_per_dim +
            ((int)(i / 3) % 3 - 1));
        size_t bin_z = mcc_utils_octree_wraparound(
            (int)(index->bin_id /
                  (octree_data.bins_per_dim * octree_data.bins_per_dim)) +
            ((int)(i / 9) - 1));

        assert(octree_data.bins_per_dim * octree_data.bins_per_dim * bin_z +
                   octree_data.bins_per_dim * bin_y + bin_x <
               octree_data.bin_count);
        octree_iter.bins[i] =
            octree_data.bins[octree_data.bins_per_dim *
                                 octree_data.bins_per_dim * bin_z +
                             octree_data.bins_per_dim * bin_y + bin_x];
    }

    mcc_Particle_Iterator_t iterator = {
        .next = mcc_data_octree_iterator_next,
        .is_done = mcc_data_octree_iterator_is_done,
    };
    return iterator;
}

//******************************************************************************
//  Internal function definitions
//******************************************************************************

static inline size_t mcc_data_octree_calc_bin(mcc_Particle_t *p) {
    return (size_t)(p->z / octree_data.bin_width) * octree_data.bin_count *
               octree_data.bin_count +
           (size_t)(p->y / octree_data.bin_width) * octree_data.bin_count +
           (size_t)(p->x / octree_data.bin_width);
}

mcc_Status_t mcc_data_octree_initalize(mcc_Config_t *config) {
    if (octree_data.is_initialized)
        return mcc_utils_status_failure("Octree instance already initialized.");

    octree_data.depth = (size_t)(config->box_length / config->cutoff_dist);
    octree_data.particle_count = config->particle_count;
    octree_data.bin_count = 1ULL << 3 * (octree_data.depth - 1);
    octree_data.bins_per_dim = 1ULL << (octree_data.depth - 1);
    octree_data.bin_size =
        2 * (size_t)(octree_data.particle_count / octree_data.bin_count + 1);
    octree_data.bin_width = config->box_length / octree_data.depth;

    octree_data.bins =
        malloc(octree_data.bin_count * sizeof(struct mcc_Octree_Particle_t **));
    assert(octree_data.bins);
    // if (octree_data.bins == NULL)
    //		mcc_panic(MCC_ERR_MALLOC, "Failed to allocate octree bin array");

    for (size_t i = 0; i < octree_data.bin_count; i++) {
        octree_data.bins[i] = malloc(octree_data.bin_size *
                                     sizeof(struct mcc_Octree_Particle_t *));
        assert(octree_data.bins[i]);
        // if (octree_data.bins[i] == NULL)
        //		mcc_panic(MCC_ERR_MALLOC, "Failed to allocate octree data
        // array");
    }

    octree_data.particles = malloc(octree_data.particle_count *
                                   sizeof(struct mcc_Octree_Particle_t));
    assert(octree_data.particles);
    // if (octree_data.particles == NULL)
    //		mcc_panic(MCC_ERR_MALLOC, "Failed to allocate particle bin array");

    srand(time(NULL));
    octree_data.is_initialized = true;
    return mcc_utils_status_success();
}

mcc_Status_t mcc_data_octree_finalize() {
    if (!octree_data.is_initialized || !octree_data.bins)
        return mcc_utils_status_failure("Octree instance is nil.");

    for (size_t i = 0; i < octree_data.bin_count; i++)
        free(octree_data.bins[i]);

    free(octree_data.bins);
    free(octree_data.particles);

    octree_data.is_initialized = false;
    return mcc_utils_status_success();
}

mcc_Particle_t *mcc_data_octree_get_particle(mcc_Index_t *index) {
    mcc_data_octree_check_initialized();
    mcc_data_octree_check_index(index);

    struct mcc_Octree_Particle_t *node =
        &octree_data.bins[index->bin_id][index->particle_id];
    return node->is_valid ? node->value : NULL;
}

bool mcc_data_octree_set_particle(mcc_Index_t *index,
                                  mcc_Particle_t *particle) {
    mcc_data_octree_check_initialized();
    mcc_data_octree_check_index(index);

    size_t bin = mcc_data_octree_calc_bin(particle);
    mcc_Index_t _index;
    if (index) {
        if (bin == index->bin_id) {
            assert(
                octree_data.bins[index->bin_id][index->particle_id].is_valid);
            *octree_data.bins[index->particle_id][index->particle_id].value =
                *particle;
            return true;
        } else {
            octree_data.bins[index->bin_id][index->particle_id].is_valid =
                false;
        }
    } else {
        assert(octree_data.fill_count < octree_data.particle_count);
        index = &_index;
        index->array_id = octree_data.fill_count++;
    }

    bool inserted = false;
    for (size_t i = 0; i < octree_data.bin_size; i++) {
        if (!octree_data.bins[bin][i].is_valid) {
            octree_data.bins[bin][i].is_valid = true;
            octree_data.particles[index->array_id] = *particle;
            octree_data.bins[bin][i].value =
                &octree_data.particles[index->array_id];
            return true;
        }
    }

    assert(inserted);
    // if (inserted == false)
    //	mcc_panic(MCC_ERR_INDEX_OUT_OF_BOUNDS,
    //	          "Aborting before octree bin overflow.");
    fprintf(stderr, "That worked!");
    return true;
}

mcc_Particle_t *mcc_data_octree_iterator_next() {
    while (octree_iter.currb < BINS_TO_COMP) {
        if (octree_iter.bins[octree_iter.currb][octree_iter.currp].is_valid)
            break;
        else {
            if (octree_iter.currp++ >= octree_data.bin_size) {
                octree_iter.currb++;
                octree_iter.currp = 0;
            }
        }
    }
    return (octree_iter.currb < BINS_TO_COMP)
               ? octree_iter.bins[octree_iter.currb][octree_iter.currp].value
               : NULL;
}

bool mcc_data_octree_iterator_is_done(mcc_Particle_t *result) {
    return result == NULL;
}

static inline size_t mcc_utils_octree_wraparound(int i) {
    return (i % octree_data.depth + octree_data.depth) % octree_data.depth;
}

void mcc_data_octree_check_index(mcc_Index_t *index) {
    if (index->bin_id >= octree_data.bin_count ||
        index->particle_id >= octree_data.bin_size) {
        char err_msg[256];
        snprintf(err_msg, sizeof(err_msg),
                 "Get-Particle: index '%zu' is out of bounds (%zu)",
                 index->array_id, octree_data.bin_count * octree_data.bin_size);
        // mcc_panic(MCC_ERR_INDEX_OUT_OF_BOUNDS, err_msg);
    }
}

void mcc_data_octree_check_initialized() {
    if (octree_data.is_initialized)
        return;
    // mcc_panic(MCC_ERR_INDEX_OUT_OF_BOUNDS,
    //         "Data has not been initialized before access");
}
*/
