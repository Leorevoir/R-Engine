import matplotlib.pyplot as plt
import numpy as np

# --- Your Benchmark Data ---
# All times are in milliseconds per frame.

entity_counts = [10000, 100000]
labels = ['10,000 Entities', '100,000 Entities']

old_model_times = [1.61761, 33.5514]
new_model_times = [0.739946, 7.85239]

# --- Calculations for the presentation ---

speedup_10k = old_model_times[0] / new_model_times[0]
speedup_100k = old_model_times[1] / new_model_times[1]

# --- Print a summary to the console ---

print("\n--- Benchmark Summary ---")
print(f"At 10,000 entities, the new model is {speedup_10k:.2f}x faster.")
print(f"At 100,000 entities, the new model is {speedup_100k:.2f}x faster.")
print("\nGenerating charts...")


# =============================================================================
# CHART 1: The Direct Comparison (Bar Chart)
# =============================================================================

x = np.arange(len(labels))  # the label locations
width = 0.35  # the width of the bars

fig1, ax1 = plt.subplots(figsize=(12, 7))

rects1 = ax1.bar(x - width/2, old_model_times, width, label='Old Model (Map-based)', color='orangered')
rects2 = ax1.bar(x + width/2, new_model_times, width, label='New Model (Archetype)', color='dodgerblue')

ax1.set_ylabel('Average Time per Frame (ms)', fontsize=12)
ax1.set_title('Benchmark: New Archetype ECS vs. Old Map-based ECS', fontsize=16, pad=20)
ax1.set_xticks(x)
ax1.set_xticklabels(labels, fontsize=12)
ax1.legend(fontsize=11)
ax1.bar_label(rects1, padding=3, fmt='%.2f ms')
ax1.bar_label(rects2, padding=3, fmt='%.2f ms')

results_text = f'Key Takeaway:\nThe new Archetype model is\n{speedup_10k:.2f}x to {speedup_100k:.2f}x faster.'
props = dict(boxstyle='round', facecolor='wheat', alpha=0.5)
ax1.text(0.95, 0.95, results_text, transform=ax1.transAxes, fontsize=12,
        verticalalignment='top', horizontalalignment='right', bbox=props)

fig1.tight_layout()
# --- MODIFICATION: Save the figure instead of showing it ---
plt.savefig('chart1_performance_comparison.png', dpi=150)
print("Chart 1 saved as 'chart1_performance_comparison.png'")


# =============================================================================
# CHART 2: The Scalability Story (Line Chart)
# =============================================================================

fig2, ax2 = plt.subplots(figsize=(12, 7))

ax2.plot(entity_counts, old_model_times, marker='o', linestyle='--', label='Old Model (Map-based)', color='orangered', markersize=8)
ax2.plot(entity_counts, new_model_times, marker='o', linestyle='-', label='New Model (Archetype)', color='dodgerblue', markersize=8)
ax2.set_yscale('log')
ax2.set_xlabel('Number of Entities', fontsize=12)
ax2.set_ylabel('Average Time per Frame (ms) - LOG SCALE', fontsize=12)
ax2.set_title('ECS Scalability: Performance Under Load', fontsize=16, pad=20)
ax2.grid(True, which="both", ls="--", c='0.7')
ax2.legend(fontsize=11)
ax2.set_xticks(entity_counts)
ax2.set_xticklabels([f'{count:,}' for count in entity_counts])

scalability_text = (
    "A flatter line means better scalability.\n\n"
    "The Old Model's performance degrades exponentially as\n"
    "entities increase, while the new Archetype model scales linearly."
)
props = dict(boxstyle='round', facecolor='lightblue', alpha=0.5)
ax2.text(0.05, 0.95, scalability_text, transform=ax2.transAxes, fontsize=12,
        verticalalignment='top', bbox=props)

fig2.tight_layout()
# --- MODIFICATION: Save the figure instead of showing it ---
plt.savefig('chart2_scalability.png', dpi=150)
print("Chart 2 saved as 'chart2_scalability.png'")
