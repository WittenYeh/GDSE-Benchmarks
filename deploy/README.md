# Deployment Scripts

This directory contains scripts for deploying benchmark visualizations to GitHub Pages.

## Quick Start

```bash
# 1. Generate plots (if not already done)
python3 visualize/plot_performance_comparison.py \
    --database neo4j,janusgraph,arangodb,aster \
    --dataset delaunay_n13,movielens-small \
    --workload small_structural_workload

# 2. Test deployment (dry run)
./deploy/test_deploy.sh

# 3. Deploy to GitHub Pages
./deploy/deploy.sh

# 4. View at: https://wittenyeh.github.io/gdse-benchmarks/
```

## Files

- **generate_index.py** - Generates a beautiful index page for all plots
- **deploy.sh** - Deploys plots to GitHub Pages

## Prerequisites

1. A GitHub Pages repository (e.g., `wittenyeh.github.io`)
2. Git configured with GitHub credentials
3. Python 3 installed
4. Generated plot files in the `plots/` directory

## Usage

### Step 1: Generate Plots

First, generate your benchmark plots using the visualization scripts:

```bash
# Performance comparison
python3 visualize/plot_performance_comparison.py \
    --database neo4j,janusgraph,arangodb,aster \
    --dataset delaunay_n13,movielens-small \
    --workload small_structural_workload

# Batch size comparison
python3 visualize/plot_batchsize_comparison.py \
    --database neo4j,janusgraph,arangodb \
    --dataset coAuthorsDBLP \
    --workload batchsize_comparison
```

### Step 2: Deploy to GitHub Pages

Deploy all plots to your GitHub Pages site:

```bash
# Deploy to default subdirectory (gdse-benchmarks)
./deploy/deploy.sh

# Or specify a custom subdirectory
./deploy/deploy.sh my-benchmarks
```

The script will:
1. ✅ Generate an index page with all plots
2. ✅ Clone your GitHub Pages repository
3. ✅ Copy all HTML files to the specified subdirectory
4. ✅ Commit and push changes
5. ✅ Display the URL where your visualizations are available

### Step 3: View Your Visualizations

After deployment, your visualizations will be available at:

```
https://wittenyeh.github.io/gdse-benchmarks/
```

Or if you specified a custom subdirectory:

```
https://wittenyeh.github.io/your-subdirectory/
```

## Features

### Beautiful Index Page

The generated index page includes:

- 📊 **Statistics Dashboard** - Shows total number of plots
- 🏆 **Performance Comparison Section** - Grouped performance plots
- 📈 **Batch Size Analysis Section** - Grouped batch size plots
- 🎨 **Modern Design** - Gradient backgrounds, hover effects, responsive layout
- 📋 **Detailed Metadata** - Dataset, workload, databases, file size for each plot
- 🔗 **Direct Links** - Click any card to view the interactive plot

### Interactive Plots

All Plotly plots maintain their full interactivity:

- Zoom and pan
- Hover tooltips
- Legend filtering
- Download as PNG
- Responsive design

## Configuration

### Customize GitHub Repository

Edit `deploy.sh` to change the target repository:

```bash
GITHUB_REPO="your-repo-name.github.io"
GITHUB_USER="your-username"
```

### Customize Index Page Style

Edit `deploy/generate_index.py` to modify:

- Colors and gradients
- Layout and spacing
- Card design
- Typography

## Troubleshooting

### "No HTML files found in plots/"

Generate plots first using the visualization scripts.

### "Failed to clone repository"

Check:
1. Repository exists and is accessible
2. Git credentials are configured (`git config --global user.name` and `user.email`)
3. You have push access to the repository

### "Failed to push to GitHub"

Ensure you have:
1. Valid GitHub authentication (SSH key or personal access token)
2. Write permissions to the repository

### GitHub Pages not updating

Wait a few minutes - GitHub Pages can take 1-5 minutes to rebuild and deploy changes.

## Example Workflow

Complete workflow from benchmark to deployment:

```bash
# 1. Run benchmarks
./benchmark.sh --database neo4j,janusgraph --dataset delaunay_n13 --workload small_structural_workload

# 2. Generate visualizations
python3 visualize/plot_performance_comparison.py \
    --database neo4j,janusgraph \
    --dataset delaunay_n13 \
    --workload small_structural_workload

# 3. Deploy to GitHub Pages
./deploy/deploy.sh

# 4. Open in browser
# Visit: https://wittenyeh.github.io/gdse-benchmarks/
```

## Notes

- The script automatically generates `index.html` in the plots directory
- All HTML files are self-contained (no external dependencies)
- Plots maintain full Plotly interactivity
- The deployment is incremental - only changed files are committed
- Old plots are not automatically deleted - manage manually if needed