#include "VirtualMemory.h"
#include "PhysicalMemory.h"
#include <cmath>


/**
 * Separates the given address into individual entries representing each subtable.
 *
 * @param address  The address to be separated.
 * @param entries  An array to store the separated entries.
 */
void separate_address(uint64_t address, uint64_t *entries) {
    for (int i = 0; i < TABLES_DEPTH; i++) {
        // Mask to extract the lower OFFSET_WIDTH bits
        uint64_t mask = (1 << OFFSET_WIDTH) - 1;
        // Get the index for the current subtable
        entries[TABLES_DEPTH - i - 1] = address & mask;
        // Shift the address to the right by OFFSET_WIDTH bits
        address >>= OFFSET_WIDTH;
    }
}

/**
 * Retrieves the sizes of each subtable in the virtual memory system.
 *
 * @param sizes  An array to store the sizes of subtables.
 */
void get_subtable_sizes(int sizes[]) {
    int root_subtable_size = (VIRTUAL_ADDRESS_WIDTH - OFFSET_WIDTH) %
                             OFFSET_WIDTH;

    sizes[0] = root_subtable_size == 0 ? OFFSET_WIDTH
                                       : root_subtable_size; // Size of the
    // root subtable

    for (int i = 1; i < TABLES_DEPTH; i++) {
        sizes[i] = OFFSET_WIDTH; // Size of subsequent subtables
    }
}

/**
 * Clears the contents of the specified frame in the physical memory.
 *
 * @param frame_number  The index of the frame to be cleared.
 */
void empty_frame(int frame_number) {
    for (int i = 0; i < PAGE_SIZE; ++i) {
        PMwrite(frame_number * PAGE_SIZE + i, 0);
    }
}

/**
 * Initializes the virtual memory system by clearing the first frame.
 */
void VMinitialize() {
    empty_frame(0);
}


/**
 * Recursively finds a frame to evict in the virtual memory system for a new page.
 *
 * @param max_used_index                        Reference to the index of the highest used frame.
 * @param free_frame_index                      Reference to store a possible free frame index.
 * @param parent_of_free_index                  Reference to store the parent frame index of the free frame.
 * @param parent_leg_free_index                 Reference to store the leg index of the free frame's parent.
 * @param maximal_cyclical_distance             Reference to the maximal cyclic distance found so far.
 * @param maximal_cyclical_distance_frame_index Reference to store the frame index with maximal cyclic distance.
 * @param maximal_cyclical_distance_page_index  Reference to store the page index with maximal cyclic distance.
 * @param parent_of_maximal_cyclical_distance   Reference to store the parent frame index of the frame with maximal cyclic distance.
 * @param parent_leg_cyclical_index             Reference to store the leg index of the frame with maximal cyclic distance's parent.
 * @param my_parent_leg                         The leg index of the current frame's parent.
 * @param current_depth                         The current depth level in the subtables hierarchy.
 * @param page_swapped_in                       The target page index being swapped in.
 * @param page_index                            The current page index.
 * @param frame_index                           The current frame index.
 * @param parent_frame_index                    The parent frame index of the current frame.
 * @param address_proportion                    An array with levels' split in the address hierarchy.
 * @param start_zero_frame                      The frame index that originally had 0 value.
 */
void find_frame_to_evict(int *max_used_index,       // Frame index highest used
                         int *free_frame_index,     //Possible free frame index
                         int *parent_of_free_index, // parent of free_frame_index
                         int *parent_leg_free_index,
                         int *maximal_cyclical_distance,
                         int *maximal_cyclical_distance_frame_index,
                         int *maximal_cyclical_distance_page_index,
                         int *parent_of_maximal_cyclical_distance,
                         int *parent_leg_cyclical_index,
                         int my_parent_leg,         // leg index of parent
                         int current_depth,
                         int page_swapped_in,       // Page target index
                         int page_index,            // current page index
                         int frame_index,           // current frame index
                         int parent_frame_index,
                         int address_proportion[],  // array with levels' split
                         int start_zero_frame       // frame index that had 0
) {
    // Check the leaf cyclic value and stop the recursion
    if (current_depth == 0) {
        int first_part = (page_swapped_in - page_index) > 0 ?
                         (page_swapped_in - page_index) :
                         (page_index - page_swapped_in);
        int second_part = NUM_PAGES - first_part;
        int cyclic_distance = first_part < second_part ? first_part :
                              second_part;
        if (cyclic_distance > *maximal_cyclical_distance) {
            *maximal_cyclical_distance = cyclic_distance;
            *maximal_cyclical_distance_frame_index = frame_index;
            *maximal_cyclical_distance_page_index = page_index;
            *parent_of_maximal_cyclical_distance = parent_frame_index;
            *parent_leg_cyclical_index = my_parent_leg;
        }
        // Find max used index
        if (frame_index > *max_used_index) {
            *max_used_index = frame_index;
        }
        return;
    }
    // Find max used index
    if (frame_index > *max_used_index) {
        *max_used_index = frame_index;
    }
    // Iterate over the children
    bool is_empty = true;
    for (int i = 0;
         i < ((1 << address_proportion[TABLES_DEPTH - current_depth]));
         i++) {
        int data;
        PMread(frame_index * PAGE_SIZE + i, &data);
        if (data != 0) {
            is_empty = false;
            int next_page_index = (page_index <<
                                              address_proportion[TABLES_DEPTH -
                                                                 current_depth]) +
                                  i;
            find_frame_to_evict(max_used_index, free_frame_index,
                                parent_of_free_index, parent_leg_free_index,
                                maximal_cyclical_distance,
                                maximal_cyclical_distance_frame_index,
                                maximal_cyclical_distance_page_index,
                                parent_of_maximal_cyclical_distance,
                                parent_leg_cyclical_index, i,
                                current_depth - 1, page_swapped_in,
                                next_page_index, data, frame_index,
                                address_proportion, start_zero_frame);
        }
    }
    // Free table frame
    if (is_empty && frame_index != start_zero_frame) {
        *free_frame_index = frame_index;
        *parent_of_free_index = parent_frame_index;
        *parent_leg_free_index = my_parent_leg;
    }
}


/**
 * Swaps a frame in the virtual memory system, either by finding a free frame or evicting one.
 *
 * @param start_zero_frame     The frame index that originally had 0 value.
 * @param page_swapped_in       The page index being swapped in.
 * @param address_proportion    An array with levels' split in the address hierarchy.
 *
 * @return The index of the swapped or evicted frame.
 */
int swap_frame(int start_zero_frame, uint64_t page_swapped_in,
               int address_proportion[]) {
    int max_used_index = -1;
    int free_frame_index = -1;
    int parent_of_free_index = -1;
    int parent_leg_free_index = -1;
    int maximal_cyclical_distance = -1;
    int maximal_cyclical_distance_frame_index = -1;
    int maximal_cyclical_distance_page_index = -1;
    int parent_of_maximal_cyclical_distance = -1;
    int parent_leg_cyclical_index = -1;

    find_frame_to_evict(&max_used_index, &free_frame_index,
                        &parent_of_free_index, &parent_leg_free_index,
                        &maximal_cyclical_distance,
                        &maximal_cyclical_distance_frame_index,
                        &maximal_cyclical_distance_page_index,
                        &parent_of_maximal_cyclical_distance,
                        &parent_leg_cyclical_index, -1, TABLES_DEPTH,
                        page_swapped_in, 0, 0, -1,
                        address_proportion, start_zero_frame);

    if (free_frame_index > 0 && parent_of_free_index > -1) {
        PMwrite(parent_of_free_index * PAGE_SIZE + parent_leg_free_index, 0);
        return free_frame_index;
    } else if (max_used_index + 1 < NUM_FRAMES) {
        return max_used_index + 1;
    } else {
        PMevict(maximal_cyclical_distance_frame_index,
                maximal_cyclical_distance_page_index);
        PMwrite(parent_of_maximal_cyclical_distance * PAGE_SIZE +
                parent_leg_cyclical_index, 0);
        return maximal_cyclical_distance_frame_index;
    }
}


/**
 * Retrieves the physical memory address corresponding to the given virtual address.
 *
 * @param virtualAddress  The virtual address to be translated.
 * @param ret_val         Reference to the return value, 1 if successful, 0 if the virtual address is out of bounds.
 *
 * @return The physical memory address corresponding to the virtual address.
 */
word_t get_PMaddress(uint64_t virtualAddress, int *ret_val) {
    if (virtualAddress >= VIRTUAL_MEMORY_SIZE) {
        *ret_val = 0;
    }

    uint64_t entries[TABLES_DEPTH];
    int address_proportion[TABLES_DEPTH];
    uint64_t page_index = (virtualAddress >> OFFSET_WIDTH) & (
        (1 << (VIRTUAL_ADDRESS_WIDTH - OFFSET_WIDTH)) - 1);
    separate_address(page_index, entries);
    // potential junk bits
    get_subtable_sizes(address_proportion);

    word_t next_read = 0;
    word_t current_frame = next_read;
    uint64_t address = 0;
    int next_free_frame;
    for (int lvl = 0; lvl < TABLES_DEPTH; lvl++) {
        address = entries[lvl];

        // try to read the next frame
        PMread(current_frame * PAGE_SIZE + address, &next_read);
        if (next_read == 0) {
            next_free_frame = swap_frame(current_frame, virtualAddress >>
                                                                       OFFSET_WIDTH,
                                         address_proportion);
            if (lvl + 1 != TABLES_DEPTH) {
                empty_frame(next_free_frame);
            }
            PMwrite(current_frame * PAGE_SIZE + address, next_free_frame);
            current_frame = next_free_frame;
        } else {
            current_frame = next_read;
        }
    }
    PMrestore(current_frame, page_index);
    return current_frame;
}


/**
 * Reads the value from the virtual memory at the specified virtual address.
 *
 * @param virtualAddress  The virtual address to read from.
 * @param value           Reference to store the retrieved value.
 *
 * @return 1 if the operation is successful, 0 if the virtual address is out of bounds.
 */
int VMread(uint64_t virtualAddress, word_t *value) {
    int ret_val = 1;
    if (virtualAddress >= VIRTUAL_MEMORY_SIZE) {
        return 0;
    }

    word_t current_frame = get_PMaddress(virtualAddress, &ret_val);
    uint64_t mask = (1 << OFFSET_WIDTH) - 1;
    uint64_t offset = virtualAddress & mask;
    PMread(current_frame * PAGE_SIZE + offset, value);
    // passed as a pointer
    return ret_val;
}


/**
 * Writes the value to the virtual memory at the specified virtual address.
 *
 * @param virtualAddress  The virtual address to write to.
 * @param value           The value to be written.
 *
 * @return 1 if the operation is successful, 0 if the virtual address is out of bounds.
 */
int VMwrite(uint64_t virtualAddress, word_t value) {
    int ret_val = 1;
    if (virtualAddress >= VIRTUAL_MEMORY_SIZE) {
        return 0;
    }

    word_t current_frame = get_PMaddress(virtualAddress, &ret_val);
    uint64_t mask = (1 << OFFSET_WIDTH) - 1;
    uint64_t offset = virtualAddress & mask;
    PMwrite(current_frame * PAGE_SIZE + offset, value);
    return ret_val;
}
