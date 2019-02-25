/*
* Copyright(c) 2019 Intel Corporation
* SPDX - License - Identifier: BSD - 2 - Clause - Patent
*/

#include <stdlib.h>
#include <string.h>

#include "EbDefinitions.h"
#include "EbPredictionStructure.h"
#include "EbUtility.h"

/**********************************************************
 * Macros
 **********************************************************/
#define PRED_STRUCT_INDEX(hierarchicalLevelCount, predType, refCount) (((hierarchicalLevelCount) * EB_PRED_TOTAL_COUNT + (predType))  + (refCount))

 /**********************************************************
  * Instructions for how to create a Predicion Structure
  *
  * Overview:
  *   The prediction structure consists of a collection
  *   of Prediction Structure Entires, which themselves
  *   consist of reference and dependent lists.  The
  *   reference lists are exactly like those found in
  *   the standard and can be clipped in order to reduce
  *   the number of references.
  *
  *   Dependent lists are the corollary to reference lists,
  *   the describe how a particular picture is referenced.
  *   Dependent lists can also be clipped at predefined
  *   junctions (i.e. the listCount array) in order
  *   to reduce the number of references.  Note that the
  *   dependent deltaPOCs must be grouped together in order
  *   of ascending referencePicture in order for the Dependent
  *   List clip to work properly.
  *
  *   All control and RPS information is derived from
  *   these lists.  The lists for a structure are defined
  *   for both P & B-picture variants.  In the case of
  *   P-pictures, only Lists 0 are used.
  *
  *   Negative deltaPOCs are for backward-referencing pictures
  *   in display order and positive deltaPOCs are for
  *   forward-referencing pictures.
  *
  *   Please note that there is no assigned coding order,
  *   the PictureManager will start pictures as soon as
  *   their references become available.
  *
  *   Any prediction structure is possible; however, we are
  *     restricting usage to the following controls:
  *     # Hierarchical Levels
  *     # Number of References
  *     # B-pictures enabled
  *     # Intra Refresh Period
  *
  *  To Get Low Delay P, only use List 0
  *  To Get Low Delay B, replace List 1 with List 0
  *  To Get Random Access, use the preduction structure as is
  **********************************************************/

  /************************************************
   * Flat
   *
   *  I-B-B-B-B-B-B-B-B
   *
   * Display & Coding Order:
   *  0 1 2 3 4 5 6 7 8
   *
   ************************************************/
static PredictionStructureConfigEntry flatPredStruct[] = {

    {
        0,              // GOP Index 0 - Temporal Layer
        0,              // GOP Index 0 - Decode Order
        1,                // GOP Index 0 - Ref List 0
        1                // GOP Index 0 - Ref List 1
    }
};

/************************************************
 * Random Access - Two-Level Hierarchical
 *
 *    b   b   b   b      Temporal Layer 1
 *   / \ / \ / \ / \
 *  I---B---B---B---B    Temporal Layer 0
 *
 * Display Order:
 *  0 1 2 3 4 5 6 7 8
 *
 * Coding Order:
 *  0 2 1 4 3 6 5 8 7
 ************************************************/
static PredictionStructureConfigEntry twoLevelHierarchicalPredStruct[] = {

    {
        0,              // GOP Index 0 - Temporal Layer
        0,              // GOP Index 0 - Decode Order
        2,                // GOP Index 0 - Ref List 0
        2                // GOP Index 0 - Ref List 1
    },

    {
        1,              // GOP Index 1 - Temporal Layer
        1,              // GOP Index 1 - Decode Order
        1,                // GOP Index 1 - Ref List 0
       -1                // GOP Index 1 - Ref List 1
    }
};

/************************************************
 * Three-Level Hierarchical
 *
 *      b   b       b   b       b   b        Temporal Layer 2
 *     / \ / \     / \ / \     / \ / \
 *    /   B   \   /   B   \   /   B   \      Temporal Layer 1
 *   /   / \   \ /   / \   \ /   / \   \
 *  I-----------B-----------B-----------B    Temporal Layer 0
 *
 * Display Order:
 *  0   1 2 3   4   5 6 7   8   9 1 1   1
 *                                0 1   2
 *
 * Coding Order:
 *  0   3 2 4   1   7 6 8   5   1 1 1   9
 *                              1 0 2
 ************************************************/
static PredictionStructureConfigEntry threeLevelHierarchicalPredStruct[] = {

    {
        0,                  // GOP Index 0 - Temporal Layer
        0,                  // GOP Index 0 - Decode Order
        4,                    // GOP Index 0 - Ref List 0
        4                    // GOP Index 0 - Ref List 1
    },

    {
        2,                  // GOP Index 1 - Temporal Layer
        2,                  // GOP Index 1 - Decode Order
        1,                     // GOP Index 1 - Ref List 0
       -1                    // GOP Index 1 - Ref List 1
    },

    {
        1,                  // GOP Index 2 - Temporal Layer
        1,                  // GOP Index 2 - Decode Order
        2,                    // GOP Index 2 - Ref List 0
       -2                    // GOP Index 2 - Ref List 1
    },

    {
        2,                   // GOP Index 3 - Temporal Layer
        3,                   // GOP Index 3 - Decode Order
        1,                     // GOP Index 3 - Ref List 0
       -1                     // GOP Index 3 - Ref List 1
    }
};

/************************************************************************************************************
 * Four-Level Hierarchical
 *
 *
 *          b     b           b     b               b     b           b     b           Temporal Layer 3
 *         / \   / \         / \   / \             / \   / \         / \   / \
 *        /   \ /   \       /   \ /   \           /   \ /   \       /   \ /   \
 *       /     B     \     /     B     \         /     B     \     /     B     \        Temporal Layer 2
 *      /     / \     \   /     / \     \       /     / \     \   /     / \     \
 *     /     /   \     \ /     /   \     \     /     /   \     \ /     /   \     \
 *    /     /     ------B------     \     \   /     /     ------B------     \     \     Temporal Layer 1
 *   /     /           / \           \     \ /     /           / \           \     \
 *  I---------------------------------------B---------------------------------------B   Temporal Layer 0
 *
 * Display Order:
 *  0       1  2  3     4     5  6  7       8       9  1  1     1     1  1  1       1
 *                                                     0  1     2     3  4  5       6
 *
 * Coding Order:
 *  0       5  3  6     2     7  4  8       1       1  1  1     1     1  1  1       9
 *                                                  3  1  4     0     5  2  6
 *
 ***********************************************************************************************************/
static PredictionStructureConfigEntry fourLevelHierarchicalPredStruct[] = {

    {
        0,                  // GOP Index 0 - Temporal Layer
        0,                  // GOP Index 0 - Decode Order
        8,     // GOP Index 0 - Ref List 0
        8     // GOP Index 0 - Ref List 1
    },

    {
        3,                  // GOP Index 1 - Temporal Layer
        3,                  // GOP Index 1 - Decode Order
        1,                    // GOP Index 1 - Ref List 0
       -1                    // GOP Index 1 - Ref List 1
    },

    {
        2,                  // GOP Index 2 - Temporal Layer
        2,                  // GOP Index 2 - Decode Order
        2,                     // GOP Index 2 - Ref List 0
       -2                     // GOP Index 2 - Ref List 1
    },

    {
        3,                  // GOP Index 3 - Temporal Layer
        4,                  // GOP Index 3 - Decode Order
        1,                    // GOP Index 3 - Ref List 0
       -1                    // GOP Index 3 - Ref List 1
    },

    {
        1,                   // GOP Index 4 - Temporal Layer
        1,                   // GOP Index 4 - Decode Order
        4,                     // GOP Index 4 - Ref List 0
       -4                     // GOP Index 4 - Ref List 1
    },

    {
        3,                  // GOP Index 5 - Temporal Layer
        6,                  // GOP Index 5 - Decode Order
        1,                     // GOP Index 5 - Ref List 0
       -1                    // GOP Index 5 - Ref List 1
    },

    {
        2,                  // GOP Index 6 - Temporal Layer
        5,                  // GOP Index 6 - Decode Order
        2,                    // GOP Index 6 - Ref List 0
       -2                    // GOP Index 6 - Ref List 1
    },

    {
        3,                  // GOP Index 7 - Temporal Layer
        7,                  // GOP Index 7 - Decode Order
        1,                    // GOP Index 7 - Ref List 0
       -1                    // GOP Index 7 - Ref List 1
    }
};

/***********************************************************************************************************
 * Five-Level Level Hierarchical
 *
 *           b     b           b     b               b     b           b     b              Temporal Layer 4
 *          / \   / \         / \   / \             / \   / \         / \   / \
 *         /   \ /   \       /   \ /   \           /   \ /   \       /   \ /   \
 *        /     B     \     /     B     \         /     B     \     /     B     \           Temporal Layer 3
 *       /     / \     \   /     / \     \       /     / \     \   /     / \     \
 *      /     /   \     \ /     /   \     \     /     /   \     \ /     /   \     \
 *     /     /     ------B------     \     \   /     /     ------B------     \     \        Temporal Layer 2
 *    /     /           / \           \     \ /     /           / \           \     \
 *   /     /           /   \-----------------B------------------   \           \     \      Temporal Layer 1
 *  /     /           /                     / \                     \           \     \
 * I-----------------------------------------------------------------------------------B    Temporal Layer 0
 *
 * Display Order:
 *  0        1  2  3     4     5  6  7       8       9  1  1     1     1  1  1         1
 *                                                      0  1     2     3  4  5         6
 *
 * Coding Order:
 *  0        9  5  1     3     1  6  1       2       1  7  1     4     1  8  1         1
 *                 0           1     2               3     4           5     6
 *
 ***********************************************************************************************************/
static PredictionStructureConfigEntry fiveLevelHierarchicalPredStruct[] = {

    {
        0,                  // GOP Index 0 - Temporal Layer
        0,                  // GOP Index 0 - Decode Order
        16,                    // GOP Index 0 - Ref List 0
        16                    // GOP Index 0 - Ref List 1
    },

    {
        4,                  // GOP Index 1 - Temporal Layer
        4,                  // GOP Index 1 - Decode Order
        1,                    // GOP Index 1 - Ref List 0
       -1                     // GOP Index 1 - Ref List 1
    },

    {
        3,                  // GOP Index 2 - Temporal Layer
        3,                  // GOP Index 2 - Decode Order
        2,                    // GOP Index 2 - Ref List 0
       -2                    // GOP Index 2 - Ref List 1
    },

    {
        4,                  // GOP Index 3 - Temporal Layer
        5,                  // GOP Index 3 - Decode Order
        1,                     // GOP Index 3 - Ref List 0
       -1                    // GOP Index 3 - Ref List 1
    },

    {
        2,                  // GOP Index 4 - Temporal Layer
        2,                  // GOP Index 4 - Decode Order
        4,                    // GOP Index 4 - Ref List 0
       -4                    // GOP Index 4 - Ref List 1
    },

    {
        4,                  // GOP Index 5 - Temporal Layer
        7,                  // GOP Index 5 - Decode Order
        1,                    // GOP Index 5 - Ref List 0
       -1                    // GOP Index 5 - Ref List 1
    },

    {
        3,                  // GOP Index 6 - Temporal Layer
        6,                  // GOP Index 6 - Decode Order
        2,                    // GOP Index 6 - Ref List 0
       -2                    // GOP Index 6 - Ref List 1
    },

    {
        4,                  // GOP Index 7 - Temporal Layer
        8,                  // GOP Index 7 - Decode Order
        1,                    // GOP Index 7 - Ref List 0
       -1                     // GOP Index 7 - Ref List 1
    },

    {
        1,                  // GOP Index 8 - Temporal Layer
        1,                  // GOP Index 8 - Decode Order
        8,                    // GOP Index 8 - Ref List 0
       -8                    // GOP Index 8 - Ref List 1
    },

    {
        4,                  // GOP Index 9 - Temporal Layer
        11,                 // GOP Index 9 - Decode Order
        1,                     // GOP Index 9 - Ref List 0
       -1                    // GOP Index 9 - Ref List 1
    },

    {
        3,                  // GOP Index 10 - Temporal Layer
        10,                 // GOP Index 10 - Decode Order
        2,                    // GOP Index 10 - Ref List 0
       -2                    // GOP Index 10 - Ref List 1
    },

    {
        4,                  // GOP Index 11 - Temporal Layer
        12,                 // GOP Index 11 - Decode Order
        1,                    // GOP Index 11 - Ref List 0
       -1                     // GOP Index 11 - Ref List 1
    },

    {
        2,                  // GOP Index 12 - Temporal Layer
        9,                  // GOP Index 12 - Decode Order
        4,                    // GOP Index 12 - Ref List 0
       -4                    // GOP Index 12 - Ref List 1
    },

    {
        4,                  // GOP Index 13 - Temporal Layer
        14,                 // GOP Index 13 - Decode Order
        1,                     // GOP Index 13 - Ref List 0
       -1                     // GOP Index 13 - Ref List 1
    },

    {
        3,                  // GOP Index 14 - Temporal Layer
        13,                 // GOP Index 14 - Decode Order
        2,                     // GOP Index 14 - Ref List 0
       -2                     // GOP Index 14 - Ref List 1
    },

    {
        4,                  // GOP Index 15 - Temporal Layer
        15,                 // GOP Index 15 - Decode Order
        1,                    // GOP Index 15 - Ref List 0
       -1                    // GOP Index 15 - Ref List 1
    }
};

/**********************************************************************************************************************************************************************************************************************
 * Six-Level Level Hierarchical
 *
 *
 *              b     b           b     b               b     b           b     b                   b     b           b     b               b     b           b     b               Temporal Layer 5
 *             / \   / \         / \   / \             / \   / \         / \   / \                 / \   / \         / \   / \             / \   / \         / \   / \
 *            /   \ /   \       /   \ /   \           /   \ /   \       /   \ /   \               /   \ /   \       /   \ /   \           /   \ /   \       /   \ /   \
 *           /     B     \     /     B     \         /     B     \     /     B     \             /     B     \     /     B     \         /     B     \     /     B     \            Temporal Layer 4
 *          /     / \     \   /     / \     \       /     / \     \   /     / \     \           /     / \     \   /     / \     \       /     / \     \   /     / \     \
 *         /     /   \     \ /     /   \     \     /     /   \     \ /     /   \     \         /     /   \     \ /     /   \     \     /     /   \     \ /     /   \     \
 *        /     /     ------B------     \     \   /     /     ------B------     \     \       /     /     ------B------     \     \   /     /     ------B------     \     \         Temporal Layer 3
 *       /     /           / \           \     \ /     /           / \           \     \     /     /           / \           \     \ /     /           / \           \     \
 *      /     /           /   \-----------------B------------------   \           \     \   /     /           /   \-----------------B------------------   \           \     \       Temporal Layer 2
 *     /     /           /                     / \                     \           \     \ /     /           /                     / \                     \           \     \
 *    /     /           /                     /   \---------------------------------------B---------------------------------------/   \                     \           \     \     Temporal Layer 1
 *   /     /           /                     /                                           / \                                           \                     \           \     \
 *  I---------------------------------------------------------------------------------------------------------------------------------------------------------------------------B   Temporal Layer 0
 *
 * Display Order:
 *  0           1  2  3     4     5  6  7       8       9  1  1     1     1  1  1         1         1  1  1     2     2  2  2       2       2  2  2     2     2  3  3           3
 *                                                         0  1     2     3  4  5         6         7  8  9     0     1  2  3       4       5  6  7     8     9  0  1           2
 *
 * Coding Order:
 *  0           1  9  1     5     1  1  2       3       2  1  2     6     2  1  2         2         2  1  2     7     2  1  2       4       2  1  3     8     3  1  3           1
 *              7     8           9  0  0               1  1  2           3  2  4                   5  3  6           7  4  8               9  5  0           1  6  2
 *
 **********************************************************************************************************************************************************************************************************************/
static PredictionStructureConfigEntry sixLevelHierarchicalPredStruct[] = {

    {
        0,                  // GOP Index 0 - Temporal Layer
        0,                  // GOP Index 0 - Decode Order
        32,                    // GOP Index 0 - Ref List 0
        32                    // GOP Index 0 - Ref List 1
    },

    {
        5,                  // GOP Index 1 - Temporal Layer
        5,                  // GOP Index 1 - Decode Order
        1,                    // GOP Index 1 - Ref List 0
       -1                    // GOP Index 1 - Ref List 1
    },

    {
        4,                  // GOP Index 2 - Temporal Layer
        4,                  // GOP Index 2 - Decode Order
        2,                    // GOP Index 2 - Ref List 0
       -2                    // GOP Index 2 - Ref List 1
    },

    {
        5,                  // GOP Index 3 - Temporal Layer
        6,                  // GOP Index 3 - Decode Order
        1,                    // GOP Index 3 - Ref List 0
       -1                    // GOP Index 3 - Ref List 1
    },

    {
        3,                  // GOP Index 4 - Temporal Layer
        3,                  // GOP Index 4 - Decode Order
        4,                    // GOP Index 4 - Ref List 0
       -4                    // GOP Index 4 - Ref List 1
    },

    {
        5,                  // GOP Index 5 - Temporal Layer
        8,                  // GOP Index 5 - Decode Order
        1,                    // GOP Index 5 - Ref List 0
       -1                    // GOP Index 5 - Ref List 1
    },

    {
        4,                  // GOP Index 6 - Temporal Layer
        7,                  // GOP Index 6 - Decode Order
        2,                    // GOP Index 6 - Ref List 0
       -2                    // GOP Index 6 - Ref List 1
    },

    {
        5,                  // GOP Index 7 - Temporal Layer
        9,                  // GOP Index 7 - Decode Order
        1,                    // GOP Index 7 - Ref List 0
       -1                    // GOP Index 7 - Ref List 1
    },

    {
        2,                  // GOP Index 8 - Temporal Layer
        2,                  // GOP Index 8 - Decode Order
        8,                    // GOP Index 8 - Ref List 0
       -8                    // GOP Index 8 - Ref List 1
    },

    {
        5,                  // GOP Index 9 - Temporal Layer
        12,                 // GOP Index 9 - Decode Order
        1,                    // GOP Index 9 - Ref List 0
       -1                    // GOP Index 9 - Ref List 1
    },

    {
        4,                  // GOP Index 10 - Temporal Layer
        11,                 // GOP Index 10 - Decode Order
         2,                    // GOP Index 10 - Ref List 0
        -2                    // GOP Index 10 - Ref List 1
    },

    {
        5,                  // GOP Index 11 - Temporal Layer
        13,                 // GOP Index 11 - Decode Order
        1,                    // GOP Index 11 - Ref List 0
       -1                    // GOP Index 11 - Ref List 1
    },

    {
        3,                  // GOP Index 12 - Temporal Layer
        10,                 // GOP Index 12 - Decode Order
        4,                    // GOP Index 12 - Ref List 0
       -4                    // GOP Index 12 - Ref List 1
    },

    {
        5,                  // GOP Index 13 - Temporal Layer
        15,                 // GOP Index 13 - Decode Order
        1,                    // GOP Index 13 - Ref List 0
       -1                    // GOP Index 13 - Ref List 1
    },

    {
        4,                  // GOP Index 14 - Temporal Layer
        14,                 // GOP Index 14 - Decode Order
        2,                    // GOP Index 14 - Ref List 0
       -2                    // GOP Index 14 - Ref List 1
    },

    {
        5,                  // GOP Index 15 - Temporal Layer
        16,                 // GOP Index 15 - Decode Order
        1,                    // GOP Index 15 - Ref List 0
       -1                    // GOP Index 15 - Ref List 1
    },

    {
        1,                  // GOP Index 16 - Temporal Layer
        1,                  // GOP Index 16 - Decode Order
        16,                    // GOP Index 16 - Ref List 0
       -16                    // GOP Index 16 - Ref List 1
    },

    {
        5,                  // GOP Index 17 - Temporal Layer
        20,                 // GOP Index 17 - Decode Order
        1,                    // GOP Index 17 - Ref List 0
       -1                    // GOP Index 17 - Ref List 1
    },

    {
        4,                  // GOP Index 18 - Temporal Layer
        19,                 // GOP Index 18 - Decode Order
        2,                    // GOP Index 18 - Ref List 0
       -2                    // GOP Index 18 - Ref List 1
    },

    {
        5,                  // GOP Index 19 - Temporal Layer
        21,                 // GOP Index 19 - Decode Order
        1,                    // GOP Index 19 - Ref List 0
       -1                    // GOP Index 19 - Ref List 1
    },

    {
        3,                  // GOP Index 20 - Temporal Layer
        18,                 // GOP Index 20 - Decode Order
         4,                    // GOP Index 20 - Ref List 0
        -4                    // GOP Index 20 - Ref List 1
    },

    {
        5,                  // GOP Index 21 - Temporal Layer
        23,                 // GOP Index 21 - Decode Order
        1,                    // GOP Index 21 - Ref List 0
       -1                     // GOP Index 21 - Ref List 1
    },

    {
        4,                  // GOP Index 22 - Temporal Layer
        22,                 // GOP Index 22 - Decode Order
        2,                    // GOP Index 22 - Ref List 0
       -2                    // GOP Index 22 - Ref List 1
    },

    {
        5,                  // GOP Index 23 - Temporal Layer
        24,                 // GOP Index 23 - Decode Order
        1,                    // GOP Index 23 - Ref List 0
       -1                    // GOP Index 23 - Ref List 1
    },

    {
        2,                  // GOP Index 24 - Temporal Layer
        17,                 // GOP Index 24 - Decode Order
        8,                    // GOP Index 24 - Ref List 0
       -8                    // GOP Index 24 - Ref List 1
    },

    {
        5,                  // GOP Index 25 - Temporal Layer
        27,                 // GOP Index 25 - Decode Order
        1,                    // GOP Index 25 - Ref List 0
       -1                    // GOP Index 25 - Ref List 1
    },

    {
        4,                  // GOP Index 26 - Temporal Layer
        26,                 // GOP Index 26 - Decode Order
        2,                    // GOP Index 26 - Ref List 0
       -2                    // GOP Index 26 - Ref List 1
    },

    {
        5,                  // GOP Index 27 - Temporal Layer
        28,                 // GOP Index 27 - Decode Order
        1,                    // GOP Index 27 - Ref List 0
       -1                    // GOP Index 27 - Ref List 1
    },

    {
        3,                  // GOP Index 28 - Temporal Layer
        25,                 // GOP Index 28 - Decode Order
         4,                    // GOP Index 28 - Ref List 0
        -4                    // GOP Index 28 - Ref List 1
    },

    {
        5,                  // GOP Index 29 - Temporal Layer
        30,                 // GOP Index 29 - Decode Order
        1,                    // GOP Index 29 - Ref List 0
       -1                    // GOP Index 29 - Ref List 1
    },

    {
        4,                  // GOP Index 30 - Temporal Layer
        29,                 // GOP Index 30 - Decode Order
        2,                    // GOP Index 30 - Ref List 0
       -2                    // GOP Index 30 - Ref List 1
    },

    {
        5,                  // GOP Index 31 - Temporal Layer
        31,                 // GOP Index 31 - Decode Order
        1,                    // GOP Index 31 - Ref List 0
       -1                    // GOP Index 31 - Ref List 1
    }
};

/************************************************
 * Prediction Structure Config Array
 ************************************************/
static const PredictionStructureConfig PredictionStructureConfigArray[] = {
    {1,     flatPredStruct},
    {2,     twoLevelHierarchicalPredStruct},
    {4,     threeLevelHierarchicalPredStruct},
    {8,     fourLevelHierarchicalPredStruct},
    {16,    fiveLevelHierarchicalPredStruct},
    {32,    sixLevelHierarchicalPredStruct},
    {0,     (PredictionStructureConfigEntry*)EB_NULL} // Terminating Code, must always come last!
};

/************************************************
 * Get Prediction Structure
 ************************************************/
PredictionStructure* get_prediction_structure(
    PredictionStructureGroup     *prediction_structure_group_ptr,
    EbPred                         pred_structure,
    uint32_t                          numberOfReferences,
    uint32_t                          levelsOfHierarchy)
{
    PredictionStructure *pred_struct_ptr;
    uint32_t pred_struct_index;

    // Convert numberOfReferences to an index
    --numberOfReferences;

    // Determine the Index value
    pred_struct_index = PRED_STRUCT_INDEX(levelsOfHierarchy, (uint32_t)pred_structure, numberOfReferences);

    pred_struct_ptr = prediction_structure_group_ptr->predictionStructurePtrArray[pred_struct_index];

    return pred_struct_ptr;
}

/********************************************************************************************
 * Prediction Structure Ctor
 *
 * GOP Type:
 *   For Low Delay P, eliminate Ref List 0
 *   Fow Low Delay B, copy Ref List 0 into Ref List 1
 *   For Random Access, leave config as is
 *
 * numberOfReferences:
 *   Clip the Ref Lists
 *
 *  Summary:
 *
 *  The Pred Struct Ctor constructs the Reference Lists, Dependent Lists, and RPS for each
 *    valid prediction structure position. The full prediction structure is composed of four
 *    sections:
 *    a. Leading Pictures
 *    b. Initialization Pictures
 *    c. Steady-state Pictures
 *    d. Trailing Pictures
 *
 *  By definition, the Prediction Structure Config describes the Steady-state Picture
 *    Set. From the PS Config, the other sections are determined by following a simple
 *    set of construction rules. These rules are:
 *    -Leading Pictures use only List 1 of the Steady-state for forward-prediction
 *    -Init Pictures don't violate CRA mechanics
 *    -Steady-state Pictures come directly from the PS Config
 *    -Following pictures use only List 0 of the Steady-state for rear-prediction
 *
 *  In general terms, Leading and Trailing pictures are useful when trying to reduce
 *    the number of base-layer pictures in the presense of scene changes.  Trailing
 *    pictures are also useful for terminating sequences.  Init pictures are needed
 *    when using multiple references that expand outside of a Prediction Structure.
 *    Steady-state pictures are the normal use cases.
 *
 *  Leading and Trailing Pictures are not applicable to Low Delay prediction structures.
 *
 *  Below are a set of example PS diagrams
 *
 *  Low-delay P, Flat, 2 reference:
 *
 *                    I---P---P
 *
 *  Display Order     0   1   2
 *
 *  Sections:
 *    Let PredStructSize = N
 *    Leading Pictures:     [null]  Size: 0
 *    Init Pictures:        [0-1]   Size: Ceil(MaxReference, N) - N + 1
 *    Stead-state Pictures: [2]     Size: N
 *    Trailing Pictures:    [null]  Size: 0
 *    ------------------------------------------
 *      Total Size: Ceil(MaxReference, N) + 1
 *
 *  Low-delay B, 3-level, 2 references:
 *
 *                         b   b     b   b
 *                        /   /     /   /
 *                       /   B     /   B
 *                      /   /     /   /
 *                     I---------B-----------B
 *
 *  Display Order      0   1 2 3 4   5 6 7   8
 *
 *  Sections:
 *    Let PredStructSize = N
 *    Leading Pictures:     [null]  Size: 0
 *    Init Pictures:        [1-4]   Size: Ceil(MaxReference, N) - N + 1
 *    Stead-state Pictures: [5-8]   Size: N
 *    Trailing Pictures:    [null]  Size: 0
 *    ------------------------------------------
 *      Total Size: Ceil(MaxReference, N) + 1
 *
 *  Random Access, 3-level structure with 3 references:
 *
 *                   p   p       b   b       b   b       b   b       p   p
 *                    \   \     / \ / \     / \ / \     / \ / \     /   /
 *                     P   \   /   B   \   /   B   \   /   B   \   /   P
 *                      \   \ /   / \   \ /   / \   \ /   / \   \ /   /
 *                       ----I-----------B-----------B-----------B----
 *  Display Order:   0 1 2   3   4 5 6   7   8 9 1   1   1 1 1   1   1 1 1
 *                                               0   1   2 3 4   5   6 7 8
 *
 *  Decode Order:    2 1 3   0   6 5 7   4   1 9 1   8   1 1 1   1   1 1 1
 *                                           0   1       4 3 5   2   6 7 8
 *
 *  Sections:
 *    Let PredStructSize = N
 *    Leading Pictures:      [0-2]   Size: N - 1
 *    Init Pictures:         [3-11]  Size: Ceil(MaxReference, N) - N + 1
 *    Steady-state Pictures: [12-15] Size: N
 *    Trailing Pictures:     [16-18] Size: N - 1
 *    ------------------------------------------
 *      Total Size: 2*N + Ceil(MaxReference, N) - 1
 *
 *  Encoding Order:
 *                   -------->----------->----------->-----------|------->
 *                                                   |           |
 *                                                   |----<------|
 *
 *
 *  Timeline:
 *
 *  The timeline is a tool that is used to determine for how long a
 *    picture should be preserved for future reference. The concept of
 *    future reference is equivalently defined as dependence.  The RPS
 *    mechanism works by signaling for each picture in the DPB whether
 *    it is used for direct reference, kept for future reference, or
 *    discarded.  The timeline merely provides a means of determing
 *    the reference picture states at each prediction structure position.
 *    Its also important to note that all signaling should be done relative
 *    to decode order, not display order. Display order is irrelevant except
 *    for signaling the POC.
 *
 *  Timeline Example: 3-Level Hierarchical with Leading and Trailing Pictures, 3 References
 *
 *                   p   p       b   b       b   b       b   b       p   p   Temporal Layer 2
 *                    \   \     / \ / \     / \ / \     / \ / \     /   /
 *                     P   \   /   B   \   /   B   \   /   B   \   /   P     Temporal Layer 1
 *                      \   \ /   / \   \ /   / \   \ /   / \   \ /   /
 *                       ----I-----------B-----------B-----------B----       Temporal Layer 0
 *
 *  Decode Order:    2 1 3   0   6 5 7   4   1 9 1   8   1 1 1   1   1 1 1
 *                                           0   1       4 3 5   2   6 7 8
 *
 *  Display Order:   0 1 2   3   4 5 6   7   8 9 1   1   1 1 1   1   1 1 1
 *                                               0   1   2 3 4   5   6 7 8
 *            X --->
 *
 *                                1 1 1 1 1 1 1 1 1   DECODE ORDER
 *            0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8
 *
 *         |----------------------------------------------------------
 *         |
 *  Y D  0 |  \ x---x-x-x-x---x-x-x---x-x-x
 *  | E  1 |    \ x
 *  | C  2 |      \
 *  | O  3 |        \
 *  v D  4 |          \ x---x-x-x-x---x-x-x---x-x-x
 *    E  5 |            \ x-x
 *       6 |              \
 *    O  7 |                \
 *    R  8 |                  \ x---x-x-x-x---x-x-x
 *    D  9 |                    \ x-x
 *    E 10 |                      \
 *    R 11 |                        \
 *      12 |                          \ x---x-x-x-x
 *      13 |                            \ x-x
 *      14 |                              \
 *      15 |                                \
 *      16 |                                  \
 *      17 |                                    \ x
 *      18 |                                      \
 *
 *  Interpreting the timeline:
 *
 *  The most important detail to keep in mind is that all signaling
 *    is done in Decode Order space. The symbols mean the following:
 *    'x' directly referenced picture
 *    '-' picture kept for future reference
 *    ' ' not referenced, inferred discard
 *    '\' eqivalent to ' ', deliminiter that nothing can be to the left of
 *
 *  The basic steps for constructing the timeline are to increment through
 *    each position in the prediction structure (Y-direction on the timeline)
 *    and mark the appropriate state: directly referenced, kept for future reference,
 *    or discarded.  As shown, base-layer pictures are referenced much more
 *    frequently than by the other layers.
 *
 *  The RPS is constructed by looking at each 'x' position in the timeline and
 *    signaling each 'y' reference as depicted in the timeline. DPB analysis is
 *    fairly straigtforward - the total number of directly-referenced and
 *    kept-for-future-reference pictures should not exceed the DPB size.
 *
 *  The RPS Ctor code follows these construction steps.
 ******************************************************************************************/
static EbErrorType PredictionStructureCtor(
    PredictionStructure       **predictionStructureDblPtr,
    const PredictionStructureConfig  *predictionStructureConfigPtr,
    EbPred                       predType,
    uint32_t                        numberOfReferences)
{
    uint32_t                  entryIndex;
    uint32_t                  configEntryIndex;
    uint32_t                  refIndex;

    // Section Variables
    uint32_t                  leadingPicCount;
    uint32_t                  initPicCount;
    uint32_t                  steadyStatePicCount;

    PredictionStructure  *predictionStructurePtr;
    EB_MALLOC(PredictionStructure*, predictionStructurePtr, sizeof(PredictionStructure), EB_N_PTR);
    *predictionStructureDblPtr = predictionStructurePtr;
    EB_MEMSET(predictionStructurePtr, 0, sizeof(PredictionStructure));

    predictionStructurePtr->predType = predType;

    // Set the Pred Struct Period
    predictionStructurePtr->predStructPeriod = predictionStructureConfigPtr->entryCount;

    //----------------------------------------
    // Find the Pred Struct Entry Count
    //   There are four sections of the pred struct:
    //     -Leading Pictures        Size: N-1
    //     -Init Pictures           Size: Ceil(MaxReference, N) - N + 1
    //     -Steady-state Pictures   Size: N
    //     -Trailing Pictures       Size: N-1
    //----------------------------------------

    //----------------------------------------
    // Determine the Prediction Structure Size
    //   First, start by determining
    //   Ceil(MaxReference, N)
    //----------------------------------------
    {
        int32_t maxRef = MIN_SIGNED_VALUE;
        for (configEntryIndex = 0, entryIndex = predictionStructureConfigPtr->entryCount - 1; configEntryIndex < predictionStructureConfigPtr->entryCount; ++configEntryIndex) {

            // Increment through Reference List 0
            refIndex = 0;
            while (refIndex < numberOfReferences && predictionStructureConfigPtr->entryArray[configEntryIndex].refList0 != 0) {
                //maxRef = MAX(predictionStructureConfigPtr->entryArray[configEntryIndex].refList0[refIndex], maxRef);
                maxRef = MAX((int32_t)(predictionStructureConfigPtr->entryCount - entryIndex - 1) + predictionStructureConfigPtr->entryArray[configEntryIndex].refList0, maxRef);
                ++refIndex;
            }

            // Increment through Reference List 1 (Random Access only)
            if (predType == EB_PRED_RANDOM_ACCESS) {
                refIndex = 0;
                while (refIndex < numberOfReferences && predictionStructureConfigPtr->entryArray[configEntryIndex].refList1 != 0) {
                    //maxRef = MAX(predictionStructureConfigPtr->entryArray[configEntryIndex].refList1[refIndex], maxRef);
                    maxRef = MAX((int32_t)(predictionStructureConfigPtr->entryCount - entryIndex - 1) + predictionStructureConfigPtr->entryArray[configEntryIndex].refList1, maxRef);
                    ++refIndex;
                }
            }

            // Increment entryIndex
            entryIndex = (entryIndex == predictionStructureConfigPtr->entryCount - 1) ? 0 : entryIndex + 1;
        }

        // Perform the Ceil(MaxReference, N) operation
        predictionStructurePtr->maximumExtent = CEILING(maxRef, predictionStructurePtr->predStructPeriod);

        // Set the Section Sizes
        leadingPicCount = (predType == EB_PRED_RANDOM_ACCESS) ?     // No leading pictures in low-delay configurations
            predictionStructurePtr->predStructPeriod - 1 :
            0;
        initPicCount = predictionStructurePtr->maximumExtent - predictionStructurePtr->predStructPeriod + 1;
        steadyStatePicCount = predictionStructurePtr->predStructPeriod;
        //trailingPicCount        = (predType == EB_PRED_RANDOM_ACCESS) ?     // No trailing pictures in low-delay configurations
        //    predictionStructurePtr->predStructPeriod - 1:
        //    0;

        // Set the total Entry Count
        predictionStructurePtr->predStructEntryCount =
            leadingPicCount +
            initPicCount +
            steadyStatePicCount;

        // Set the Section Indices
        predictionStructurePtr->leadingPicIndex = 0;
        predictionStructurePtr->initPicIndex = predictionStructurePtr->leadingPicIndex + leadingPicCount;
        predictionStructurePtr->steadyStateIndex = predictionStructurePtr->initPicIndex + initPicCount;
    }

    // Allocate the entry array
    EB_MALLOC(PredictionStructureEntry**, predictionStructurePtr->predStructEntryPtrArray, sizeof(PredictionStructureEntry*) * predictionStructurePtr->predStructEntryCount, EB_N_PTR);
    // Allocate the entries
    for (entryIndex = 0; entryIndex < predictionStructurePtr->predStructEntryCount; ++entryIndex) {
        EB_MALLOC(PredictionStructureEntry*, predictionStructurePtr->predStructEntryPtrArray[entryIndex], sizeof(PredictionStructureEntry), EB_N_PTR);
        EB_MEMSET(predictionStructurePtr->predStructEntryPtrArray[entryIndex], 0, sizeof(PredictionStructureEntry));
    }

    // Find the Max Temporal Layer Index
    predictionStructurePtr->temporalLayerCount = 0;
    for (configEntryIndex = 0; configEntryIndex < predictionStructureConfigPtr->entryCount; ++configEntryIndex) {
        predictionStructurePtr->temporalLayerCount = MAX(predictionStructureConfigPtr->entryArray[configEntryIndex].temporal_layer_index, predictionStructurePtr->temporalLayerCount);
    }

    // Increment the Zero-indexed temporal layer index to get the total count
    ++predictionStructurePtr->temporalLayerCount;

    //----------------------------------------
    // Construct Leading Pictures
    //   -Use only Ref List1 from the Config
    //   -Note the Config starts from the 2nd position to construct the leading pictures
    //----------------------------------------
    {
        for (entryIndex = 0, configEntryIndex = 1; entryIndex < leadingPicCount; ++entryIndex, ++configEntryIndex) {

            // Find the Size of the Config's Reference List 1
            refIndex = 0;
            while (refIndex < numberOfReferences && predictionStructureConfigPtr->entryArray[configEntryIndex].refList1 != 0) {
                ++refIndex;
            }

            // Set Leading Picture's Reference List 0 Count {Config List1 => LeadingPic List 0}
            predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList0.referenceListCount = refIndex;

            // Allocate the Leading Picture Reference List 0
            predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList0.referenceList = 0;

            // Copy Config List1 => LeadingPic Reference List 0
            for (refIndex = 0; refIndex < predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList0.referenceListCount; ++refIndex) {
                predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList0.referenceList = predictionStructureConfigPtr->entryArray[configEntryIndex].refList1;
            }

            // Null out List 1
            predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList1.referenceListCount = 0;
            predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList1.referenceList = 0;

            // Set the Temporal Layer Index
            predictionStructurePtr->predStructEntryPtrArray[entryIndex]->temporal_layer_index = predictionStructureConfigPtr->entryArray[configEntryIndex].temporal_layer_index;

            // Set the Decode Order
            predictionStructurePtr->predStructEntryPtrArray[entryIndex]->decode_order = (predType == EB_PRED_RANDOM_ACCESS) ?
                predictionStructureConfigPtr->entryArray[configEntryIndex].decode_order :
                entryIndex;

        }
    }

    //----------------------------------------
    // Construct Init Pictures
    //   -Use only references from Ref List0 & Ref List1 from the Config that don't violate CRA mechanics
    //   -The Config Index cycles through continuously
    //----------------------------------------
    {
        uint32_t terminatingEntryIndex = entryIndex + initPicCount;
        int32_t pocValue;

        for (configEntryIndex = 0, pocValue = 0; entryIndex < terminatingEntryIndex; ++entryIndex, ++pocValue) {

            // REFERENCE LIST 0

            // Find the Size of the Config's Reference List 0
            refIndex = 0;
            while (
                refIndex < numberOfReferences &&
                predictionStructureConfigPtr->entryArray[configEntryIndex].refList0 != 0 &&
                pocValue - predictionStructureConfigPtr->entryArray[configEntryIndex].refList0 >= 0)  // Stop when we violate the CRA (i.e. reference past it)
            {
                ++refIndex;
            }

            // Set Leading Picture's Reference List 0 Count
            predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList0.referenceListCount = refIndex;

            // Allocate the Leading Picture Reference List 0{
            predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList0.referenceList = 0;

            // Copy Reference List 0
            for (refIndex = 0; refIndex < predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList0.referenceListCount; ++refIndex) {
                predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList0.referenceList = predictionStructureConfigPtr->entryArray[configEntryIndex].refList0;
            }

            // REFERENCE LIST 1
            switch (predType) {

            case EB_PRED_LOW_DELAY_P:

                // Null out List 1
                predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList1.referenceListCount = 0;
                predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList1.referenceList = 0;

                break;

            case EB_PRED_LOW_DELAY_B:

                // Copy List 0 => List 1

                // Set Leading Picture's Reference List 1 Count
                predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList1.referenceListCount = predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList0.referenceListCount;

                // Allocate the Leading Picture Reference List 1
                predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList1.referenceList = 0;


                // Copy Reference List 1
                for (refIndex = 0; refIndex < predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList1.referenceListCount; ++refIndex) {
                    predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList1.referenceList = predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList0.referenceList;
                }

                break;

            case EB_PRED_RANDOM_ACCESS:

                // Find the Size of the Config's Reference List 1
                refIndex = 0;
                while (
                    refIndex < numberOfReferences &&
                    predictionStructureConfigPtr->entryArray[configEntryIndex].refList1 != 0 &&
                    pocValue - predictionStructureConfigPtr->entryArray[configEntryIndex].refList1 >= 0) // Stop when we violate the CRA (i.e. reference past it)
                {
                    ++refIndex;
                }

                // Set Leading Picture's Reference List 1 Count
                predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList1.referenceListCount = refIndex;

                // Allocate the Leading Picture Reference List 1
                predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList1.referenceList = 0;


                // Copy Reference List 1
                for (refIndex = 0; refIndex < predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList1.referenceListCount; ++refIndex) {
                    predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList1.referenceList = predictionStructureConfigPtr->entryArray[configEntryIndex].refList1;
                }

                break;

            default:
                break;

            }

            // Set the Temporal Layer Index
            predictionStructurePtr->predStructEntryPtrArray[entryIndex]->temporal_layer_index = predictionStructureConfigPtr->entryArray[configEntryIndex].temporal_layer_index;

            // Set the Decode Order
            predictionStructurePtr->predStructEntryPtrArray[entryIndex]->decode_order = (predType == EB_PRED_RANDOM_ACCESS) ?
                predictionStructureConfigPtr->entryArray[configEntryIndex].decode_order :
                entryIndex;

            // Rollover the Config Index
            configEntryIndex = (configEntryIndex == predictionStructureConfigPtr->entryCount - 1) ?
                0 :
                configEntryIndex + 1;
        }
    }

    //----------------------------------------
    // Construct Steady-state Pictures
    //   -Copy directly from the Config
    //----------------------------------------
    {
        uint32_t terminatingEntryIndex = entryIndex + steadyStatePicCount;

        for (/*configEntryIndex = 0*/; entryIndex < terminatingEntryIndex; ++entryIndex/*, ++configEntryIndex*/) {

            // Find the Size of Reference List 0
            refIndex = 0;
            while (refIndex < numberOfReferences && predictionStructureConfigPtr->entryArray[configEntryIndex].refList0 != 0) {
                ++refIndex;
            }

            // Set Reference List 0 Count
            predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList0.referenceListCount = refIndex;

            // Allocate Reference List 0
            //EB_MALLOC(int32_t*, predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList0.referenceList, sizeof(int32_t) * predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList0.referenceListCount, EB_N_PTR);
            // Copy Reference List 0
            for (refIndex = 0; refIndex < predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList0.referenceListCount; ++refIndex) {
                predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList0.referenceList = predictionStructureConfigPtr->entryArray[configEntryIndex].refList0;
            }

            // REFERENCE LIST 1
            switch (predType) {

            case EB_PRED_LOW_DELAY_P:

                // Null out List 1
                predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList1.referenceListCount = 0;
                predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList1.referenceList = 0;

                break;

            case EB_PRED_LOW_DELAY_B:

                // Copy List 0 => List 1

                // Set Leading Picture's Reference List 1 Count
                predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList1.referenceListCount = predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList0.referenceListCount;

                // Allocate the Leading Picture Reference List 1
                predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList1.referenceList = 0;


                // Copy Reference List 1
                for (refIndex = 0; refIndex < predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList1.referenceListCount; ++refIndex) {
                    predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList1.referenceList = predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList0.referenceList;
                }

                break;

            case EB_PRED_RANDOM_ACCESS:

                // Find the Size of the Config's Reference List 1
                refIndex = 0;
                while (refIndex < numberOfReferences && predictionStructureConfigPtr->entryArray[configEntryIndex].refList1 != 0) {
                    ++refIndex;
                }

                // Set Leading Picture's Reference List 1 Count
                predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList1.referenceListCount = refIndex;

                // Allocate the Leading Picture Reference List 1
                predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList1.referenceList = 0;


                // Copy Reference List 1
                for (refIndex = 0; refIndex < predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList1.referenceListCount; ++refIndex) {
                    predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList1.referenceList = predictionStructureConfigPtr->entryArray[configEntryIndex].refList1;
                }

                break;

            default:
                break;

            }

            // Set the Temporal Layer Index
            predictionStructurePtr->predStructEntryPtrArray[entryIndex]->temporal_layer_index = predictionStructureConfigPtr->entryArray[configEntryIndex].temporal_layer_index;

            // Set the Decode Order
            predictionStructurePtr->predStructEntryPtrArray[entryIndex]->decode_order = (predType == EB_PRED_RANDOM_ACCESS) ?
                predictionStructureConfigPtr->entryArray[configEntryIndex].decode_order :
                entryIndex;

            // Rollover the Config Index
            configEntryIndex = (configEntryIndex == predictionStructureConfigPtr->entryCount - 1) ?
                0 :
                configEntryIndex + 1;
        }
    }

    //----------------------------------------
    // Construct Trailing Pictures
    //   -Use only Ref List0 from the Config
    //----------------------------------------
    //{
    //    uint32_t terminatingEntryIndex = entryIndex + trailingPicCount;
    //
    //    for(configEntryIndex = 0; entryIndex < terminatingEntryIndex; ++entryIndex, ++configEntryIndex) {
    //
    //        // Set Reference List 0 Count
    //        // *Note - only 1 reference is used for trailing pictures.  If you have frequent CRAs and the Pred Struct
    //        //   has many references, you can run into edge conditions.
    //        predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList0.referenceListCount = 1;
    //
    //        // Allocate Reference List 0
    //        predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList0.referenceList = (int32_t*) malloc(sizeof(int32_t) * predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList0.referenceListCount);
    //
    //        // Copy Reference List 0
    //        for(refIndex = 0; refIndex < predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList0.referenceListCount; ++refIndex) {
    //            predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList0.referenceList[refIndex] = predictionStructureConfigPtr->entryArray[configEntryIndex].refList0[refIndex];
    //        }
    //
    //        // Null out List 1
    //        predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList1.referenceListCount = 0;
    //        predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList1.referenceList = (int32_t*) EB_NULL;
    //
    //        // Set the Temporal Layer Index
    //        predictionStructurePtr->predStructEntryPtrArray[entryIndex]->temporal_layer_index = predictionStructureConfigPtr->entryArray[configEntryIndex].temporal_layer_index;
    //
    //        // Set the Decode Order
    //        predictionStructurePtr->predStructEntryPtrArray[entryIndex]->decode_order = (predType == EB_PRED_RANDOM_ACCESS) ?
    //            predictionStructureConfigPtr->entryArray[configEntryIndex].decode_order :
    //            entryIndex;
    //    }
    //}

    //----------------------------------------
    // CONSTRUCT DEPENDENT LIST 0
    //----------------------------------------

    {
        int64_t  depIndex;
        uint64_t  picture_number;

        // First, determine the Dependent List Size for each Entry by incrementing the dependent list length
        {

            // Go through a single pass of the Leading Pictures and Init pictures
            for (picture_number = 0, entryIndex = 0; picture_number < predictionStructurePtr->steadyStateIndex; ++picture_number) {

                // Go through each Reference picture and accumulate counts
                for (refIndex = 0; refIndex < predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList0.referenceListCount; ++refIndex) {

                    depIndex = picture_number - predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList0.referenceList;

                    if (depIndex >= 0 && depIndex < (int32_t)(predictionStructurePtr->steadyStateIndex + predictionStructurePtr->predStructPeriod)) {
                        ++predictionStructurePtr->predStructEntryPtrArray[depIndex]->depList0.listCount;
                    }
                }

                // Increment the entryIndex
                ++entryIndex;
            }

            // Go through an entire maximum extent pass for the Steady-state pictures
            for (entryIndex = predictionStructurePtr->steadyStateIndex; picture_number <= predictionStructurePtr->steadyStateIndex + 2 * predictionStructurePtr->maximumExtent; ++picture_number) {

                // Go through each Reference picture and accumulate counts
                for (refIndex = 0; refIndex < predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList0.referenceListCount; ++refIndex) {

                    depIndex = picture_number - predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList0.referenceList;

                    if (depIndex >= 0 && depIndex < (int32_t)(predictionStructurePtr->steadyStateIndex + predictionStructurePtr->predStructPeriod)) {
                        ++predictionStructurePtr->predStructEntryPtrArray[depIndex]->depList0.listCount;
                    }
                }

                // Rollover the entryIndex each time it reaches the end of the steady state index
                entryIndex = (entryIndex == predictionStructurePtr->predStructEntryCount - 1) ?
                    predictionStructurePtr->steadyStateIndex :
                    entryIndex + 1;
            }
        }

        // Second, allocate memory for each dependent list of each Entry
        for (entryIndex = 0; entryIndex < predictionStructurePtr->predStructEntryCount; ++entryIndex) {

            // If the dependent list count is non-zero, allocate the list, else the list is NULL.
            if (predictionStructurePtr->predStructEntryPtrArray[entryIndex]->depList0.listCount > 0) {
                EB_MALLOC(int32_t*, predictionStructurePtr->predStructEntryPtrArray[entryIndex]->depList0.list, sizeof(int32_t) * predictionStructurePtr->predStructEntryPtrArray[entryIndex]->depList0.listCount, EB_N_PTR);
            }
            else {
                predictionStructurePtr->predStructEntryPtrArray[entryIndex]->depList0.list = (int32_t*)EB_NULL;
            }
        }

        // Third, reset the Dependent List Length (they are re-derived)
        for (entryIndex = 0; entryIndex < predictionStructurePtr->predStructEntryCount; ++entryIndex) {
            predictionStructurePtr->predStructEntryPtrArray[entryIndex]->depList0.listCount = 0;
        }

        // Fourth, run through each Reference List entry again and populate the Dependent Lists and Dep List Counts
        {
            // Go through a single pass of the Leading Pictures and Init pictures
            for (picture_number = 0, entryIndex = 0; picture_number < predictionStructurePtr->steadyStateIndex; ++picture_number) {

                // Go through each Reference picture and accumulate counts
                for (refIndex = 0; refIndex < predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList0.referenceListCount; ++refIndex) {

                    depIndex = picture_number - predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList0.referenceList;

                    if (depIndex >= 0 && depIndex < (int32_t)(predictionStructurePtr->steadyStateIndex + predictionStructurePtr->predStructPeriod)) {
                        predictionStructurePtr->predStructEntryPtrArray[depIndex]->depList0.list[predictionStructurePtr->predStructEntryPtrArray[depIndex]->depList0.listCount++] =
                            predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList0.referenceList;
                    }
                }

                // Increment the entryIndex
                ++entryIndex;
            }

            // Go through an entire maximum extent pass for the Steady-state pictures
            for (entryIndex = predictionStructurePtr->steadyStateIndex; picture_number <= predictionStructurePtr->steadyStateIndex + 2 * predictionStructurePtr->maximumExtent; ++picture_number) {

                // Go through each Reference picture and accumulate counts
                for (refIndex = 0; refIndex < predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList0.referenceListCount; ++refIndex) {

                    depIndex = picture_number - predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList0.referenceList;

                    // Assign the Reference to the Dep List and Increment the Dep List Count
                    if (depIndex >= 0 && depIndex < (int32_t)(predictionStructurePtr->steadyStateIndex + predictionStructurePtr->predStructPeriod)) {
                        predictionStructurePtr->predStructEntryPtrArray[depIndex]->depList0.list[predictionStructurePtr->predStructEntryPtrArray[depIndex]->depList0.listCount++] =
                            predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList0.referenceList;
                    }
                }

                // Rollover the entryIndex each time it reaches the end of the steady state index
                entryIndex = (entryIndex == predictionStructurePtr->predStructEntryCount - 1) ?
                    predictionStructurePtr->steadyStateIndex :
                    entryIndex + 1;
            }
        }
    }

    //----------------------------------------
    // CONSTRUCT DEPENDENT LIST 1
    //----------------------------------------

    {
        int32_t  depIndex;
        uint32_t  picture_number;

        // First, determine the Dependent List Size for each Entry by incrementing the dependent list length
        {

            // Go through a single pass of the Leading Pictures and Init pictures
            for (picture_number = 0, entryIndex = 0; picture_number < predictionStructurePtr->steadyStateIndex; ++picture_number) {

                // Go through each Reference picture and accumulate counts
                for (refIndex = 0; refIndex < predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList1.referenceListCount; ++refIndex) {

                    depIndex = picture_number - predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList1.referenceList;

                    if (depIndex >= 0 && depIndex < (int32_t)(predictionStructurePtr->steadyStateIndex + predictionStructurePtr->predStructPeriod)) {
                        ++predictionStructurePtr->predStructEntryPtrArray[depIndex]->depList1.listCount;
                    }
                }

                // Increment the entryIndex
                ++entryIndex;
            }

            // Go through an entire maximum extent pass for the Steady-state pictures
            for (entryIndex = predictionStructurePtr->steadyStateIndex; picture_number <= predictionStructurePtr->steadyStateIndex + 2 * predictionStructurePtr->maximumExtent; ++picture_number) {

                // Go through each Reference picture and accumulate counts
                for (refIndex = 0; refIndex < predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList1.referenceListCount; ++refIndex) {

                    depIndex = picture_number - predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList1.referenceList;

                    if (depIndex >= 0 && depIndex < (int32_t)(predictionStructurePtr->steadyStateIndex + predictionStructurePtr->predStructPeriod)) {
                        ++predictionStructurePtr->predStructEntryPtrArray[depIndex]->depList1.listCount;
                    }
                }

                // Rollover the entryIndex each time it reaches the end of the steady state index
                entryIndex = (entryIndex == predictionStructurePtr->predStructEntryCount - 1) ?
                    predictionStructurePtr->steadyStateIndex :
                    entryIndex + 1;
            }
        }

        // Second, allocate memory for each dependent list of each Entry
        for (entryIndex = 0; entryIndex < predictionStructurePtr->predStructEntryCount; ++entryIndex) {

            // If the dependent list count is non-zero, allocate the list, else the list is NULL.
            if (predictionStructurePtr->predStructEntryPtrArray[entryIndex]->depList1.listCount > 0) {
                EB_MALLOC(int32_t*, predictionStructurePtr->predStructEntryPtrArray[entryIndex]->depList1.list, sizeof(int32_t) * predictionStructurePtr->predStructEntryPtrArray[entryIndex]->depList1.listCount, EB_N_PTR);
            }
            else {
                predictionStructurePtr->predStructEntryPtrArray[entryIndex]->depList1.list = (int32_t*)EB_NULL;
            }
        }

        // Third, reset the Dependent List Length (they are re-derived)
        for (entryIndex = 0; entryIndex < predictionStructurePtr->predStructEntryCount; ++entryIndex) {
            predictionStructurePtr->predStructEntryPtrArray[entryIndex]->depList1.listCount = 0;
        }

        // Fourth, run through each Reference List entry again and populate the Dependent Lists and Dep List Counts
        {
            // Go through a single pass of the Leading Pictures and Init pictures
            for (picture_number = 0, entryIndex = 0; picture_number < predictionStructurePtr->steadyStateIndex; ++picture_number) {

                // Go through each Reference picture and accumulate counts
                for (refIndex = 0; refIndex < predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList1.referenceListCount; ++refIndex) {

                    depIndex = picture_number - predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList1.referenceList;

                    if (depIndex >= 0 && depIndex < (int32_t)(predictionStructurePtr->steadyStateIndex + predictionStructurePtr->predStructPeriod)) {
                        predictionStructurePtr->predStructEntryPtrArray[depIndex]->depList1.list[predictionStructurePtr->predStructEntryPtrArray[depIndex]->depList1.listCount++] =
                            predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList1.referenceList;
                    }
                }

                // Increment the entryIndex
                ++entryIndex;
            }

            // Go through an entire maximum extent pass for the Steady-state pictures
            for (entryIndex = predictionStructurePtr->steadyStateIndex; picture_number <= predictionStructurePtr->steadyStateIndex + 2 * predictionStructurePtr->maximumExtent; ++picture_number) {

                // Go through each Reference picture and accumulate counts
                for (refIndex = 0; refIndex < predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList1.referenceListCount; ++refIndex) {

                    depIndex = picture_number - predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList1.referenceList;

                    // Assign the Reference to the Dep List and Increment the Dep List Count
                    if (depIndex >= 0 && depIndex < (int32_t)(predictionStructurePtr->steadyStateIndex + predictionStructurePtr->predStructPeriod)) {
                        predictionStructurePtr->predStructEntryPtrArray[depIndex]->depList1.list[predictionStructurePtr->predStructEntryPtrArray[depIndex]->depList1.listCount++] =
                            predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList1.referenceList;
                    }
                }

                // Rollover the entryIndex each time it reaches the end of the steady state index
                entryIndex = (entryIndex == predictionStructurePtr->predStructEntryCount - 1) ?
                    predictionStructurePtr->steadyStateIndex :
                    entryIndex + 1;
            }
        }
    }

    // Set isReferenced for each entry
    for (entryIndex = 0; entryIndex < predictionStructurePtr->predStructEntryCount; ++entryIndex) {
        predictionStructurePtr->predStructEntryPtrArray[entryIndex]->isReferenced =
            ((predictionStructurePtr->predStructEntryPtrArray[entryIndex]->depList0.listCount > 0) ||
            (predictionStructurePtr->predStructEntryPtrArray[entryIndex]->depList1.listCount > 0)) ?
            EB_TRUE :
            EB_FALSE;
    }

    //----------------------------------------
    // CONSTRUCT THE RPSes
    //----------------------------------------
    {
        // Counts & Indicies
        uint32_t      refIndex;
        uint32_t      depIndex;
        uint32_t      entryIndex;
        uint32_t      currentPocIndex;
        uint32_t      refPocIndex;

        uint32_t      decodeOrderTableSize;
        int32_t     *decodeOrderTable;
        uint32_t     *displayOrderTable;
        uint32_t      gopNumber;
        uint32_t      baseNumber;

        // Timeline Map Variables
        EbBool    *timelineMap;
        uint32_t      timelineSize;

        int32_t      depListMax;
        int32_t      depListMin;

        int32_t      lifetimeStart;
        int32_t      lifetimeSpan;

        int32_t      deltaPoc;
        int32_t      prevDeltaPoc;
        EbBool     pocInReferenceList0;
        EbBool     pocInReferenceList1;
        EbBool     pocInTimeline;

        int32_t      adjustedDepIndex;

        // Allocate & Initialize the Timeline map
        timelineSize = predictionStructurePtr->predStructEntryCount;
        decodeOrderTableSize = CEILING(predictionStructurePtr->predStructEntryCount + predictionStructurePtr->maximumExtent, predictionStructurePtr->predStructEntryCount);
        EB_MALLOC(EbBool*, timelineMap, sizeof(EbBool) * SQR(timelineSize), EB_N_PTR);
        EB_MEMSET(timelineMap, 0, sizeof(EbBool) * SQR(timelineSize));

        // Construct the Decode & Display Order
        EB_MALLOC(int32_t*, decodeOrderTable, sizeof(int32_t) * decodeOrderTableSize, EB_N_PTR);

        EB_MALLOC(uint32_t*, displayOrderTable, sizeof(uint32_t) * decodeOrderTableSize, EB_N_PTR);

        for (currentPocIndex = 0, entryIndex = 0; currentPocIndex < decodeOrderTableSize; ++currentPocIndex) {

            // Set the Decode Order
            gopNumber = (currentPocIndex / predictionStructurePtr->predStructPeriod);
            baseNumber = gopNumber * predictionStructurePtr->predStructPeriod;

            if (predType == EB_PRED_RANDOM_ACCESS) {
                decodeOrderTable[currentPocIndex] = baseNumber + predictionStructurePtr->predStructEntryPtrArray[entryIndex]->decode_order;
            }
            else {
                decodeOrderTable[currentPocIndex] = currentPocIndex;
            }
            displayOrderTable[decodeOrderTable[currentPocIndex]] = currentPocIndex;

            // Increment the entryIndex
            entryIndex = (entryIndex == predictionStructurePtr->predStructEntryCount - 1) ?
                predictionStructurePtr->predStructEntryCount - predictionStructurePtr->predStructPeriod :
                entryIndex + 1;
        }

        // Construct the timeline map from the dependency lists
        for (refPocIndex = 0, entryIndex = 0; refPocIndex < timelineSize; ++refPocIndex) {

            // Initialize Max to most negative signed value and Min to most positive signed value
            depListMax = MIN_SIGNED_VALUE;
            depListMin = MAX_SIGNED_VALUE;

            // Find depListMax and depListMin for the entryIndex in the prediction structure for DepList0
            for (depIndex = 0; depIndex < predictionStructurePtr->predStructEntryPtrArray[entryIndex]->depList0.listCount; ++depIndex) {

                adjustedDepIndex = predictionStructurePtr->predStructEntryPtrArray[entryIndex]->depList0.list[depIndex] + (int32_t)refPocIndex;

                //if(adjustedDepIndex >= 0 && adjustedDepIndex < (int32_t) timelineSize) {
                if (adjustedDepIndex >= 0) {

                    // Update Max
                    depListMax = MAX(decodeOrderTable[adjustedDepIndex], depListMax);

                    // Update Min
                    depListMin = MIN(decodeOrderTable[adjustedDepIndex], depListMin);
                }
            }

            // Continue search for depListMax and depListMin for the entryIndex in the prediction structure for DepList1,
            //   the lists are combined in the RPS logic
            for (depIndex = 0; depIndex < predictionStructurePtr->predStructEntryPtrArray[entryIndex]->depList1.listCount; ++depIndex) {

                adjustedDepIndex = predictionStructurePtr->predStructEntryPtrArray[entryIndex]->depList1.list[depIndex] + (int32_t)refPocIndex;

                //if(adjustedDepIndex >= 0 && adjustedDepIndex < (int32_t) timelineSize)  {
                if (adjustedDepIndex >= 0) {

                    // Update Max
                    depListMax = MAX(decodeOrderTable[adjustedDepIndex], depListMax);

                    // Update Min
                    depListMin = MIN(decodeOrderTable[adjustedDepIndex], depListMin);
                }

            }

            // If the Dependent Lists are empty, ensure that no RPS signaling is set
            if ((predictionStructurePtr->predStructEntryPtrArray[entryIndex]->depList0.listCount > 0) ||
                (predictionStructurePtr->predStructEntryPtrArray[entryIndex]->depList1.listCount > 0)) {

                // Determine lifetimeStart and lifetimeSpan - its important to note that out-of-range references are
                //   clipped/eliminated to not violate IDR/CRA referencing rules
                lifetimeStart = depListMin;

                if (lifetimeStart < (int32_t)timelineSize) {
                    lifetimeStart = CLIP3(0, (int32_t)(timelineSize - 1), lifetimeStart);

                    lifetimeSpan = depListMax - depListMin + 1;
                    lifetimeSpan = CLIP3(0, (int32_t)timelineSize - lifetimeStart, lifetimeSpan);

                    // Set the timelineMap
                    for (currentPocIndex = (uint32_t)lifetimeStart; currentPocIndex < (uint32_t)(lifetimeStart + lifetimeSpan); ++currentPocIndex) {
                        timelineMap[refPocIndex*timelineSize + displayOrderTable[currentPocIndex]] = EB_TRUE;
                    }
                }
            }

            // Increment the entryIndex
            entryIndex = (entryIndex == predictionStructurePtr->predStructEntryCount - 1) ?
                predictionStructurePtr->predStructEntryCount - predictionStructurePtr->predStructPeriod :
                entryIndex + 1;
        }

        //--------------------------------------------------------
        // Create the RPS for Prediction Structure Entry
        //--------------------------------------------------------

        // *Note- many of the below Syntax Elements are signaled
        //    in the Slice Header and not in the RPS.  These syntax
        //    elements can be configured during runtime to manipulate
        //    existing RPS structures.  E.g. a reference list could
        //    be shortened...

        // Initialize the RPS Group
        predictionStructurePtr->restrictedRefPicListsEnableFlag = EB_TRUE;
        predictionStructurePtr->listsModificationEnableFlag = EB_FALSE;
        predictionStructurePtr->longTermEnableFlag = EB_FALSE;
        predictionStructurePtr->defaultRefPicsList0TotalCountMinus1 = 0;
        predictionStructurePtr->defaultRefPicsList1TotalCountMinus1 = 0;

        // For each RPS Index
        for (entryIndex = 0; entryIndex < predictionStructurePtr->predStructEntryCount; ++entryIndex) {

            // Determine the Current POC Index
            currentPocIndex = entryIndex;

            // Initialize the RPS
            predictionStructurePtr->predStructEntryPtrArray[entryIndex]->shortTermRpsInSpsFlag = EB_TRUE;
            predictionStructurePtr->predStructEntryPtrArray[entryIndex]->shortTermRpsInSpsIndex = entryIndex;
            predictionStructurePtr->predStructEntryPtrArray[entryIndex]->interRpsPredictionFlag = EB_FALSE;
            predictionStructurePtr->predStructEntryPtrArray[entryIndex]->longTermRpsPresentFlag = EB_FALSE;
            predictionStructurePtr->predStructEntryPtrArray[entryIndex]->list0ModificationFlag = EB_FALSE;
            predictionStructurePtr->predStructEntryPtrArray[entryIndex]->negativeRefPicsTotalCount = 0;
            predictionStructurePtr->predStructEntryPtrArray[entryIndex]->positiveRefPicsTotalCount = 0;
            predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refPicsList0TotalCountMinus1 = ~0;
            predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refPicsList1TotalCountMinus1 = ~0;

            // Create the Negative List
            prevDeltaPoc = 0;
            for (refPocIndex = currentPocIndex - 1; (int32_t)refPocIndex >= 0; --refPocIndex) {

                // Find the deltaPoc value
                deltaPoc = (int32_t)currentPocIndex - (int32_t)refPocIndex;

                // Check to see if the deltaPoc is in Reference List 0
                pocInReferenceList0 = EB_FALSE;
                for (refIndex = 0; (refIndex < predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList0.referenceListCount) && (pocInReferenceList0 == EB_FALSE); ++refIndex) {

                    // Reference List 0
                    if ((predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList0.referenceList != 0) &&
                        (predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList0.referenceList == deltaPoc))
                    {
                        pocInReferenceList0 = EB_TRUE;
                        ++predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refPicsList0TotalCountMinus1;
                    }
                }

                // Check to see if the deltaPoc is in Reference List 1
                pocInReferenceList1 = EB_FALSE;
                for (refIndex = 0; (refIndex < predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList1.referenceListCount) && (pocInReferenceList1 == EB_FALSE); ++refIndex) {

                    // Reference List 1
                    if ((predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList1.referenceList != 0) &&
                        (predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList1.referenceList == deltaPoc))
                    {
                        pocInReferenceList1 = EB_TRUE;
                        ++predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refPicsList1TotalCountMinus1;
                    }
                }

                // Check to see if the refPocIndex is in the timeline
                pocInTimeline = timelineMap[refPocIndex*timelineSize + currentPocIndex];

                // If the deltaPoc is in the timeline
                if (pocInTimeline == EB_TRUE) {
                    predictionStructurePtr->predStructEntryPtrArray[entryIndex]->usedByNegativeCurrPicFlag[predictionStructurePtr->predStructEntryPtrArray[entryIndex]->negativeRefPicsTotalCount] = (pocInReferenceList0 == EB_TRUE || pocInReferenceList1 == EB_TRUE) ? EB_TRUE : EB_FALSE;
                    predictionStructurePtr->predStructEntryPtrArray[entryIndex]->deltaNegativeGopPosMinus1[predictionStructurePtr->predStructEntryPtrArray[entryIndex]->negativeRefPicsTotalCount++] = deltaPoc - 1 - prevDeltaPoc;
                    prevDeltaPoc = deltaPoc;
                }
            }

            // Create the Positive List
            prevDeltaPoc = 0;
            for (refPocIndex = currentPocIndex + 1; refPocIndex < timelineSize; ++refPocIndex) {

                // Find the deltaPoc value
                deltaPoc = (int32_t)currentPocIndex - (int32_t)refPocIndex;

                // Check to see if the deltaPoc is in Ref List 0
                pocInReferenceList0 = EB_FALSE;
                for (refIndex = 0; (refIndex < predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList0.referenceListCount) && (pocInReferenceList0 == EB_FALSE); ++refIndex) {

                    // Reference List 0
                    if ((predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList0.referenceList != 0) &&
                        (predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList0.referenceList == deltaPoc))
                    {
                        pocInReferenceList0 = EB_TRUE;
                        ++predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refPicsList0TotalCountMinus1;
                    }
                }

                // Check to see if the deltaPoc is in Ref List 1
                pocInReferenceList1 = EB_FALSE;
                for (refIndex = 0; (refIndex < predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList1.referenceListCount) && (pocInReferenceList1 == EB_FALSE); ++refIndex) {
                    if ((predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList1.referenceList != 0) &&
                        (predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refList1.referenceList == deltaPoc))
                    {
                        pocInReferenceList1 = EB_TRUE;
                        ++predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refPicsList1TotalCountMinus1;
                    }
                }

                // Check to see if the Y-position is in the timeline
                pocInTimeline = timelineMap[refPocIndex*timelineSize + currentPocIndex];

                // If the Y-position is in the time lime
                if (pocInTimeline == EB_TRUE) {
                    predictionStructurePtr->predStructEntryPtrArray[entryIndex]->usedByPositiveCurrPicFlag[predictionStructurePtr->predStructEntryPtrArray[entryIndex]->positiveRefPicsTotalCount] = (pocInReferenceList0 == EB_TRUE || pocInReferenceList1 == EB_TRUE) ? EB_TRUE : EB_FALSE;
                    predictionStructurePtr->predStructEntryPtrArray[entryIndex]->deltaPositiveGopPosMinus1[predictionStructurePtr->predStructEntryPtrArray[entryIndex]->positiveRefPicsTotalCount++] = -deltaPoc - 1 - prevDeltaPoc;
                    prevDeltaPoc = -deltaPoc;
                }
            }

            // Adjust Reference Counts if list is empty
            predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refPicsList0TotalCountMinus1 =
                (predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refPicsList0TotalCountMinus1 == ~0) ?
                0 :
                predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refPicsList0TotalCountMinus1;

            predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refPicsList1TotalCountMinus1 =
                (predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refPicsList1TotalCountMinus1 == ~0) ?
                0 :
                predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refPicsList1TotalCountMinus1;

            // Set refPicsOverrideTotalCountFlag to TRUE if RefListCount is different than the default
            predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refPicsOverrideTotalCountFlag =
                ((predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refPicsList0TotalCountMinus1 != (int32_t)predictionStructurePtr->defaultRefPicsList0TotalCountMinus1) ||
                (predictionStructurePtr->predStructEntryPtrArray[entryIndex]->refPicsList1TotalCountMinus1 != (int32_t)predictionStructurePtr->defaultRefPicsList1TotalCountMinus1)) ?
                EB_TRUE :
                EB_FALSE;

        }

        // Free the decode order table
        //free(decodeOrderTable);

        // Free the display order table
        //free(displayOrderTable);

        // Free the timeline map
        //free(timelineMap);
    }

    return EB_ErrorNone;
}

/*************************************************
 * Prediction Structure Group Ctor
 *
 * Summary: Converts the Prediction Structure Config
 *   into the usable Prediction Structure with RPS and
 *   Dependent List control.
 *
 * From each config, several prediction structures
 *   are created. These include:
 *   -Variable Number of References
 *      # [1 - 4]
 *   -Temporal Layers
 *      # [1 - 6]
 *   -GOP Type
 *      # Low Delay P
 *      # Low Delay B
 *      # Random Access
 *
 *************************************************/
EbErrorType prediction_structure_group_ctor(
    PredictionStructureGroup   **predictionStructureGroupDblPtr,
    uint32_t                         base_layer_switch_mode)
{
    uint32_t          pred_struct_index = 0;
    uint32_t          refIdx = 0;
    uint32_t          hierarchicalLevelIdx;
    uint32_t          predTypeIdx;
    uint32_t          numberOfReferences;
    EbErrorType    return_error = EB_ErrorNone;

    PredictionStructureGroup *prediction_structure_group_ptr;
    EB_MALLOC(PredictionStructureGroup*, prediction_structure_group_ptr, sizeof(PredictionStructureGroup), EB_N_PTR);
    *predictionStructureGroupDblPtr = prediction_structure_group_ptr;

    // Count the number of Prediction Structures
    while ((PredictionStructureConfigArray[pred_struct_index].entryArray != 0) && (PredictionStructureConfigArray[pred_struct_index].entryCount != 0)) {
        // Get Random Access + P for temporal ID 0
        if (PredictionStructureConfigArray[pred_struct_index].entryArray->temporal_layer_index == 0 && base_layer_switch_mode) {
            PredictionStructureConfigArray[pred_struct_index].entryArray->refList1 = 0;
        }
        ++pred_struct_index;
    }

    prediction_structure_group_ptr->predictionStructureCount = MAX_TEMPORAL_LAYERS * EB_PRED_TOTAL_COUNT;
    EB_MALLOC(PredictionStructure**, prediction_structure_group_ptr->predictionStructurePtrArray, sizeof(PredictionStructure*) * prediction_structure_group_ptr->predictionStructureCount, EB_N_PTR);
    for (hierarchicalLevelIdx = 0; hierarchicalLevelIdx < MAX_TEMPORAL_LAYERS; ++hierarchicalLevelIdx) {
        for (predTypeIdx = 0; predTypeIdx < EB_PRED_TOTAL_COUNT; ++predTypeIdx) {
            pred_struct_index = PRED_STRUCT_INDEX(hierarchicalLevelIdx, predTypeIdx, refIdx);
            numberOfReferences = refIdx + 1;

            return_error = PredictionStructureCtor(
                &(prediction_structure_group_ptr->predictionStructurePtrArray[pred_struct_index]),
                &(PredictionStructureConfigArray[hierarchicalLevelIdx]),
                (EbPred)predTypeIdx,
                numberOfReferences);
            if (return_error == EB_ErrorInsufficientResources) {
                return EB_ErrorInsufficientResources;
            }
        }
    }

    return EB_ErrorNone;
}

