#ifndef TA_OPTIMIZER_H
#define TA_OPTIMIZER_H

#include "matrix.h"

typedef enum optimizer_type{
    GD
} optimizer_type_t;

void update(void);

void gradient_descent_update(void);

#endif