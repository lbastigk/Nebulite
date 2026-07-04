# Nebulite Preprocessor Defines

## NEBULITE_USE_INVOKE_CONTAINER_STRINGMAP

Use the custom `StringMap` implementation for the broadcast listen container instead of the default `absl::flat_hash_map`.
Stringmap might offer performance benefits for a large number of topics (untested).