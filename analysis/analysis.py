import pandas as pd
import plotly.graph_objects as go
from plotly.subplots import make_subplots
import argparse

# Parse command-line arguments
parser = argparse.ArgumentParser(description="Plot sensor data from a CSV file.")
parser.add_argument("filename", help="Path to the CSV file")
args = parser.parse_args()

filename = args.filename
df = pd.read_csv(filename)

print(df.head())
print(df.info())
print(df.describe())

colors = [
    '#1f77b4', '#ff7f0e', '#2ca02c', '#d62728',
    '#9467bd', '#8c564b', '#e377c2', '#7f7f7f',
    '#bcbd22', '#17becf'
]

fig = make_subplots(
    rows=2, 
    cols=1, 
    subplot_titles=("Sensor Readings", "Differentiated Readings")
)

# Add traces for all columns to the first subplot
for index, col in enumerate(df.columns):
    fig.add_trace(
        go.Scatter(
            x=df.index,
            y=df[col],
            mode='lines',
            name=col,
            legendgroup=col,  # Group legend by column
            line=dict(color=colors[index])
        ),
        row=1, col=1
    )

# Add traces for all Differentiations to the second subplot
for index, col in enumerate(df.columns):
    fig.add_trace(
        go.Scatter(
            x=df.index,
            y=df[col].diff(),
            mode='lines',
            name=col,
            legendgroup=col,  # Group legend by column
            showlegend=False,  # Hide duplicate legend entries
            line=dict(color=colors[index])  # Use dashed lines for differentiations
        ),
        row=2, col=1
    )

# Update layout
fig.update_layout(
    title=f"{filename}",
    showlegend=True,
)

# Format x-axis for time display
fig.update_xaxes(
    tickangle=45  # Rotate ticks
)

# Update y-axis labels
fig.update_yaxes(title_text="Sensor Readings", row=1, col=1)
fig.update_yaxes(title_text="Differentiated Readings", row=2, col=1)

# Show figure
fig.show()