# Optimization Value Analysis

This report presents a series of test paths, the computed optimization values (`OptiValue`) for different threshold parameters `x`, and an interpretation of how key factors—delivery count, hub visits, fuel costs, and revisit penalties—drive the final score.

## Test Paths
```
6  # number of test paths
5
0 2 4 6 8
5
0 1 2 11 3
7
0 1 2 4 5 4 6
8
0 1 3 1 3 2 10 2
6
0 10 2 11 4 12
9
0 1 2 1 2 3 10 2 11
```
- **Path 1 (`0 2 4 6 8`)**: Pure hub traversal (4 hubs), no stations or houses, no revisits.
- **Path 2 (`0 1 2 11 3`)**: Visits hub 0, delivers to house 1, hub 2, refuel at station 11, delivers to house 3; no revisits.
- **Path 3 (`0 1 2 4 5 4 6`)**: Similar to Path 2 plus an extra hub 4 stop and a revisit of hub 4 (1 revisit penalty).
- **Path 4 (`0 1 3 1 3 2 10 2`)**: Multiple house visits (1 & 3) with repeated loops between hubs (3 revisits).
- **Path 5 (`0 10 2 11 4 12`)**: Hub–station–hub–station sequence, no house deliveries, no revisits.
- **Path 6 (`0 1 2 1 2 3 10 2 11`)**: Complex loop delivering to houses 1 & 3 with three revisits and final refuel.

## Results for Different `x` Values
All runs (x = 0…5) produce identical `OptiValue` results because **no path achieves delivered > x**, so the formula falls back to `cost/visited + penalty` every time.

```
=== x = 0…5 ===
Path 1: OptiValue = 5.50    (cost=22, visited=4)
Path 2: OptiValue = 22.00   (cost=22, visited=1)
Path 3: OptiValue ≈ 24.67   (cost=44, visited=3, penalty=1)
Path 4: OptiValue = 30.00   (cost=0,  visited=1, penalty=3)
Path 5: OptiValue = 0.00    (cost=0,  visited=2)
Path 6: OptiValue = 30.00   (cost=0,  visited=1, penalty=3)
```

## Discussion and Detailed Analysis
1. **Influence of `x` (Delivered > x branch)**  
   - Triggered when `delivered > x`.  
   - Formula: `cost * (visited/delivered) + penalty`.  
   - *Example (hypothetical Path 7):* If a route had delivered=2, visited=1, cost=20, penalty=0 and x=1, OptiValue = 20*(1/2)=10.

2. **Fallback for `delivered <= x` (Visited > 0 branch)**  
   - Fires when `delivered <= x` but at least one hub visited.  
   - Formula: `cost/visited + penalty`.  
   - Observed in Paths 1–6 (all delivered=0).  
   - *Example (Path 2):* cost=22, visited=1 → 22/1=22.

3. **Fallback for `visited = 0` (No hubs visited)**  
   - Occurs when no hubs were visited.  
   - Formula: `cost + penalty`.  
   - *Example (Path 8 hypothetical):* A pure station-only or house-only route with cost=15, penalty=2 → OptiValue = 17.

4. **Revisit Penalty Impact**  
   - Each extra node revisit adds 10 points.  
   - Path 3: one revisit → 44/3 ≈14.67 + 10 = 24.67.  
   - Path 4 & 6: three revisits → penalty=30, dominating zero-cost loops.

5. **Fuel Costs Breakdown**  
   - Components: `initial` (to first station), `last` (from last station), `total` (sum).  
   - Full cost term in formula is `(total + initial + last)`.  
   - *Path 1:* total=11, initial=0, last=11 → cost = 22.  
   - *Path 5:* two stations but no actual driving loop → cost = 0.

6. **Routing Recommendations**  
   - **Maximize deliveries:** Craft routes delivering more houses per hub to activate the high-efficiency branch.  
   - **Avoid revisits:** Even short loops can rack up penalties, overshadowing cost savings.  
   - **Balance fuel segments:** Splitting long distances by strategically placed stations reduces both initial and last costs.  
   - **Ensure hub visits when delivering:** A house stop without its hub yields no delivery credit and can trigger the `visited=0` fallback.

---
*Next Steps:* incorporate real delivery paths (where delivered > 0), adjust `x` thresholds, or tweak penalty weighting to reflect operational priorities.

