# METHOD
| Code | Name Method | Impact |
|---|---|---|
| M001 | Baseline Optimized | standard optimized |
| M002 | Lookup Table | good for branch prediction |
| M003 | SWAR (SIMD with a register) | good for batch processing |
| M004 | Arena Pool | good for storing processed data |
| M005 | Lookup Table + SWAR | |
| M006 | Lookup Table + Arena Pool | |

# DEPENDENCIES

```mermaid 
graph TD

	A["Baseline Optimezed"] -- "true child" --> B["Lookup Table"]
	A -- "true child" --> C["SWAR (SIMD with a register)"]
	C -- "true child" --> D["Arena Pool"]
	B -- "combine" --> C
	C -- "child of SWAR + Lookup Table" --> E["Lookup Table + SWAR"]
	B -- "combine" --> D
	D -- "child of Arena Pool + Lookup Table" --> F["Lookup Table + Arena Pool"]
```