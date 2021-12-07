#include "cache-sim.h"

#include "memalloc.h"
#include <stdlib.h>
#include <stddef.h>
#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

/** Create and return a new cache-simulation structure for a
 *  cache for main memory withe the specified cache parameters params.
 *  No guarantee that *params is valid after this call.
 */
CacheSim *
new_cache_sim(const CacheParams *params) {
    CacheSim* sim = mallocChk(sizeof(CacheSim));
    sim->nSetBits = params->nSetBits;
    sim->nLinesPerSet = params->nLinesPerSet;
    sim->nLineBits = params->nLineBits;
    sim->nMemAddrBits = params->nMemAddrBits;
    sim->replacement = params->replacement;

    //cache malloc
    unsigned long** sets = mallocChk( (1 << params->nSetBits) * sizeof(unsigned long*));
    for(int i=0; i < (1 << params->nSetBits); i++) {
        sets[i] = callocChk(1, params->nLinesPerSet * sizeof(unsigned long));
    }
    sim->cache = sets;

    //valid cache malloc
    int** validBitArr = mallocChk((1 << params->nSetBits) * sizeof(int*));
    for(int i=0; i < (1 << params->nSetBits); i++) {
        validBitArr[i] = callocChk(1, params->nLinesPerSet * sizeof(int));
    }
    sim->cacheValid = validBitArr;

    //valid cache init
    for(int i=0; i < (1 << sim->nSetBits); i++) {
        for (int j = 0; j < sim->nLinesPerSet; j++) {
            sim->cacheValid[i][j] = 0;
        }
    }

    //Cache Age malloc
    time_t** cacheAge = mallocChk((1 << params->nSetBits) * sizeof(time_t*));
    for(int i=0; i < (1 << params->nSetBits); i++) {
        cacheAge[i] = callocChk(1, params->nLinesPerSet * sizeof(time_t));
    }
    sim->cacheAge = cacheAge;
    //Cache age init
    for(int i=0; i < (1 << sim->nSetBits); i++) {
        for (int j = 0; j < sim->nLinesPerSet; j++) {
            sim->cacheAge[i][j] = 0;
        }
    }

    return sim;
}

/** Free all resources used by cache-simulation structure *cache */
void
free_cache_sim(CacheSim *cache){
    for(int i=0; i < (1 << cache->nSetBits); i++) {
        free(cache->cache[i]);
        free(cache->cacheValid[i]);
        free(cache->cacheAge[i]);
    }
    free(cache->cache);
    free(cache->cacheValid);
    free(cache->cacheAge);
    free(cache);
}
MemAddr removeb_bits(MemAddr address, unsigned nLineBits) {
    MemAddr mask = ULONG_MAX;
    mask = mask << nLineBits;
    MemAddr ret = address & mask;
    return ret;
}

MemAddr getTagBits(MemAddr address, unsigned tagBitsSize, unsigned nMemAddrBits) {
    MemAddr ret = address >> (nMemAddrBits - tagBitsSize);
    return ret;
}

MemAddr getSetBits(MemAddr address, unsigned nSetBits, unsigned nMemAddrBits, unsigned tagBits) {
    MemAddr ret = address >> (nMemAddrBits - tagBits - nSetBits);
    MemAddr mask = 0;
    mask = (1 << nSetBits) - 1;
    ret = ret & mask;
    return ret;
}
//shift to right by b bits and then mask
/** Return result for requesting addr from cache */
CacheResult
cache_sim_result(CacheSim *cache, MemAddr addr) {
    CacheResult result_hit = { CACHE_HIT, 0 };
    CacheResult result_miss_noReplace = { CACHE_MISS_WITHOUT_REPLACE, 0 };
    // cache replacement strategies

    unsigned tagBitsSize = cache->nMemAddrBits - (cache->nLineBits + cache->nSetBits);
    unsigned addressSet = getSetBits(addr, cache->nSetBits, cache->nMemAddrBits, tagBitsSize);

    //Hit - found in cache
        for(int j=0; j < cache->nLinesPerSet; j++) {
            if ((getTagBits(cache->cache[addressSet][j], tagBitsSize, cache->nMemAddrBits) == getTagBits(addr, tagBitsSize, cache->nMemAddrBits)
                && (cache->cacheValid[addressSet][j]))) {
                    sleep(1);
                    cache->cacheAge[addressSet][j] = time(NULL);
                    return result_hit;
            }
        }
        //cache hit fails - look for miss w/o replacement - populate free cache lines
        for(int j=0; j < cache->nLinesPerSet; j++) {
            if(!cache->cacheValid[addressSet][j]){
                cache->cacheValid[addressSet][j] = 1;
                cache->cache[addressSet][j] = removeb_bits(addr, cache->nLineBits);
                sleep(1);
                cache->cacheAge[addressSet][j] = time(NULL);
                return result_miss_noReplace;
            }
        }
        //if both hit and miss w/o replacement fail - use replacement strategy
        if(cache->replacement == LRU_R) {
            time_t min = LONG_MAX;
            int tempIndex = -1;
            for(int j=0; j < cache->nLinesPerSet; j++) {
                if(cache->cacheAge[addressSet][j] < min) {
                    min = cache->cacheAge[addressSet][j];
                    tempIndex = j;
                }
            }
            MemAddr replacedAddress = cache->cache[addressSet][tempIndex];
            cache->cache[addressSet][tempIndex] = removeb_bits(addr, cache->nLineBits);
            sleep(1);
            cache->cacheAge[addressSet][tempIndex] = time(NULL);
            CacheResult result_miss_replace = { CACHE_MISS_WITH_REPLACE, replacedAddress};
            return result_miss_replace;
        } else if(cache->replacement == MRU_R) {
            time_t max = LONG_MIN;
            int tempIndex = -1;
            for(int j=0; j < cache->nLinesPerSet; j++) {
                if(cache->cacheAge[addressSet][j] > max) {
                    max = cache->cacheAge[addressSet][j];
                    tempIndex = j;
                }
            }
            MemAddr replacedAddress = cache->cache[addressSet][tempIndex];
            cache->cache[addressSet][tempIndex] = removeb_bits(addr, cache->nLineBits);
            sleep(1);
            cache->cacheAge[addressSet][tempIndex] = time(NULL);
            CacheResult result_miss_replace = { CACHE_MISS_WITH_REPLACE, replacedAddress};
            return result_miss_replace;
        } else if(cache->replacement == RANDOM_R) {
            unsigned index = rand() % cache->nLinesPerSet;
            MemAddr replacedAddress = cache->cache[addressSet][index];
            cache->cache[addressSet][index] = removeb_bits(addr, cache->nLineBits);
            //cache->cacheAge[addressSet][index] = time(NULL);
            CacheResult result_miss_replace = { CACHE_MISS_WITH_REPLACE, replacedAddress};
            return result_miss_replace;
        }
        
        //0xabcd - least signifcant b bits - 8 bits - cd
        //         s bit - b
        //         t bits - a
        //0xab10  - b=10, s=b, t=a
        //0x1b10  - b=10, s=b, t=1
        //0x1c10  - b=10, s=c, t=1
        //8-2-4-16
        //0xbb10  - b=10, s=b, t=b

}
