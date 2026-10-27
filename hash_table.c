// Really dumb string-to-int hash table. Terrible hash function, linear probing,
// but can be swapped out later.

const int HASH_TABLE_INITIAL_ELEMENTS = 8;

typedef struct hash_entry {
    string key;
    uint32_t value;
    bool is_used;
} hash_entry;

typedef struct hash_table {
    hash_entry *entries;
    int num_entries_allocated;
    int num_entries_used;
} hash_table;

int hash(string key) {
    int result = 0;
    for (uint64_t i = 0; i < key.length; i++) {
        char c = key.buf[i];
        result += c;
    }
    return result;
}

void hash_table_expand_if_necessary(hash_table *table);

// Remember to call hash_table_free
hash_table hash_table_alloc() {
    return (hash_table){
        .num_entries_allocated = HASH_TABLE_INITIAL_ELEMENTS,
        .num_entries_used = 0,
        .entries = xalloc(HASH_TABLE_INITIAL_ELEMENTS * sizeof(hash_entry)),
    };
}

void hash_table_insert(hash_table *table, string key, uint32_t value) {
    int position = hash(key) % table->num_entries_allocated;
    while (table->entries[position].is_used) {
        if (string_equals(key, table->entries[position].key)) {
            // Found existing; update instead of insert
            table->entries[position].value = value;
            return;
        }
        position = (position + 1) % table->num_entries_allocated;
    }

    table->entries[position] = (hash_entry){
        .is_used = true,
        .key = key,
        .value = value,
    };
    table->num_entries_used += 1;
    hash_table_expand_if_necessary(table);
}

// Return whether 'key' is in table, possibly storing its value in 'out'
bool hash_table_lookup(hash_table *table, string key, uint32_t *out) {
    int position = hash(key) % table->num_entries_allocated;
    while (table->entries[position].is_used) {
        hash_entry entry = table->entries[position];
        if (string_equals(key, entry.key)) {
            *out = entry.value;
            return true;
        }
        position = (position + 1) % table->num_entries_allocated;
    }

    return false;
}

// (don't need to call this outside of this file)
void hash_table_expand_if_necessary(hash_table *table) {
    if (table->num_entries_used < table->num_entries_allocated / 2) {
        return;
    }

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

void hash_table_free(hash_table table) {
    xfree(table.entries);
}

void hash_table_print(hash_table table) {
    printf("================= HASH TABLE =================\n");
    for (int i = 0; i < table.num_entries_allocated; i++) {
        hash_entry entry = table.entries[i];
        if (entry.is_used) {
            printf("| value=%4u | key=\"", entry.value);
            print_string(entry.key);
            printf("\"\n");
        }
    }
    printf("=============== END HASH TABLE ===============\n");
}

void hash_table_test() {
    hash_table table = hash_table_alloc();

    hash_table_insert(&table, string_from_cstring("Hello"), 10);
    hash_table_insert(&table, string_from_cstring("Sam"), 25);
    hash_table_insert(&table, string_from_cstring("sammykablammy"), 42);
    uint32_t res;
    bool found_it = hash_table_lookup(&table, string_from_cstring("Sam"), &res);
    printf("Found it=%d Lookup: %u\n", found_it, res);
    found_it = hash_table_lookup(&table, string_from_cstring("Hello2"), &res);
    printf("Found it=%d Lookup: %u\n", found_it, res);
    hash_table_insert(&table, string_from_cstring("bruh 1"), 123);
    hash_table_insert(&table, string_from_cstring("bruh 2"), 1000);
    hash_table_insert(&table, string_from_cstring("bruh 3"), 2000);
    hash_table_insert(&table, string_from_cstring("bruh 4"), 40000);
    found_it = hash_table_lookup(&table, string_from_cstring("bruh 4"), &res);
    printf("Found it=%d Lookup: %u\n", found_it, res);
    hash_table_insert(&table, string_from_cstring("bruh 5"), 6969);
    hash_table_insert(&table, string_from_cstring("bruh 6"), 6767);
    hash_table_print(table);

    hash_table_free(table);
}
