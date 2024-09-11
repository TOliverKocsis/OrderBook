# This script file plots the price change of the ExampleDataset.csv file, that's created by DataGenerator.py

import matplotlib.pyplot as plt
import pandas as pd

# Chose theme/style for generated plot
# plt.style.use('Solarize_Light2')
plt.style.use('seaborn-v0_8')

# Read the CSV file, and report errors
file_path = '../ExampleOrderDataset/ExampleDataset.csv'  # Replace with your actual file path
try:
    df = pd.read_csv(file_path)
except FileNotFoundError:
    print("Error: The file at {file_path} was not found.")
    exit(1)
except pd.errors.EmptyDataError:
    print("Error: The file is empty.")
    exit(1)
except pd.errors.ParserError:
    print("Error: The file could not be parsed. Please check the file format.")
    exit(1)
except Exception as e:
    print(f"An unexpected error occurred: {e}")
    exit(1)

# Filter out rows where Price is <null>
df = df.dropna(subset=['Price'])

# Separate the data for 'buy' and 'sell' order types
buy_orders = df[df['Order Type'] == 'buy']
sell_orders = df[df['Order Type'] == 'sell']

# Plot the price changes
plt.figure(figsize=(12, 6))

plt.plot(buy_orders['Order ID'], buy_orders['Price'], label='Buy Orders', color='green', alpha=0.5)
plt.plot(sell_orders['Order ID'], sell_orders['Price'], label='Sell Orders', color='violet', alpha=0.4)

# Add labels and title
plt.xlabel('Order ID')
plt.ylabel('Price')
plt.title('Price Changes for Buy and Sell Orders')
plt.legend()

plt.tight_layout()  # reduce white space / focuses in

# Save the plot as an image file
# plt.show()
output_path = '../ExampleOrderDataset/buynsell_price_plot.png'
plt.savefig(output_path)

print(f"Plot saved as {output_path}")
