#pragma once

//-----------------------------------------
// INVOKE THREADING SETTINGS


//#define THREADED_MIN_BATCHSIZE 200  // Based on benchmark tests. Last check: 2025-07
#define THREADED_MIN_BATCHSIZE 8000  // Making sure only one thread is used for tests

//-----------------------------------------
// RENDERER UPDATE THREADING SETTINGS

//#define BATCH_COST_GOAL 500
#define BATCH_COST_GOAL 50000