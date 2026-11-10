// Really dumb string-to-blob hash table. Terrible hash function, linear
// probing, but can be swapped out later.

const int HASH_TABLE_INITIAL_ELEMENTS = 8;

typedef struct hash_entry {
    string key;
    // void *value;
    uint8_t value[128]; // Arbitrarily chosen to be a big number, larger than
                        // any actual value would be. this is imperfect but i
                        // don't want to have any macro magic. Alternative is to
                        // have a separate buffer 'hash_table.values' to go with
                        // the existing 'entries' buffer
    bool is_used;
} hash_entry;

typedef struct hash_table {
    hash_entry *entries;
    int num_entries_allocated;
    int num_entries_used;
    size_t bytes_per_element;
} hash_table;

int hash(string key) {
    int result = 0;
    for (uint64_t i = 0; i < key.length; i++) {
        char c = key.buf[i];
        result += c;
    }
    return result;
}

// Remember to call hash_table_free
hash_table hash_table_alloc(size_t bytes_per_element) {
    return (hash_table){
        .num_entries_allocated = HASH_TABLE_INITIAL_ELEMENTS,
        .num_entries_used = 0,
        .entries = xalloc(HASH_TABLE_INITIAL_ELEMENTS * sizeof(hash_entry)),
        .bytes_per_element = bytes_per_element,
    };
}

void hash_table_insert(hash_table *table, string key, void *value) {
    int position = hash(key) % table->num_entries_allocated;
    while (table->entries[position].is_used) {
        if (string_equals(key, table->entries[position].key)) {
            // Found existing; update instead of insert
            memcpy(&table->entries[position].value, value, table->bytes_per_element);
            return;
        }
        position = (position + 1) % table->num_entries_allocated;
    }

    table->entries[position] = (hash_entry){
        .is_used = true,
        .key = key,
    };
    memcpy(&table->entries[position].value, value, table->bytes_per_element);
    table->num_entries_used += 1;

    if (table->num_entries_used < table->num_entries_allocated / 2) {
        return;
    }
    // EXPAND
    int old_num_entries_allocated = table->num_entries_allocated;
    hash_entry *old_entries = table->entries;
    table->num_entries_used = 0;
    table->num_entries_allocated *= 2;
    table->entries = xalloc(table->num_entries_allocated * sizeof(hash_entry));
    for (int i = 0; i < old_num_entries_allocated; i++) {
        if (old_entries[i].is_used) {
            hash_table_insert(table, old_entries[i].key, old_entries[i].value);
        }
    }
    xfree(old_entries);
}

// Return whether 'key' is in table, possibly storing its value in 'out'
bool hash_table_lookup(hash_table *table, string key, uint32_t *out) {
    int position = hash(key) % table->num_entries_allocated;
    while (table->entries[position].is_used) {
        hash_entry entry = table->entries[position];
        if (string_equals(key, entry.key)) {
            memcpy(out, entry.value, table->bytes_per_element);
            return true;
        }
        position = (position + 1) % table->num_entries_allocated;
    }

    return false;
}


void hash_table_free(hash_table table) {
    xfree(table.entries);
}

void hash_table_print(hash_table table, const char *name) {
    printf("================= %s =================\n", name);
    for (int i = 0; i < table.num_entries_allocated; i++) {
        hash_entry entry = table.entries[i];
        if (entry.is_used) {
            printf("| value=%4lu | key=\"", (uintptr_t)entry.value);
            print_string(entry.key);
            printf("\"\n");
        }
    }
    printf("=============== END %s ===============\n", name);
}
