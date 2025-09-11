import pandas as pd

# Define column names according to LOBSTER documentation
column_names = [
    "Time",         # Seconds after midnight (float)
    "EventType",    # 1-7 event type
    "OrderID",      # Unique order ID
    "Size",         # Shares
    "Price",        # Price in $ * 10000
    "Direction"     # -1 = sell, 1 = buy
]

# File paths – adjust if needed
file_message_1 = "/Users/iffathsalah/Developer/cpp-orderbook/input-data/AMZN_2012-06-21_34200000_57600000_message_1.csv"
file_message_10 = "/Users/iffathsalah/Developer/cpp-orderbook/input-data/AMZN_2012-06-21_34200000_57600000_message_10.csv"

# Read both files
try:
    df_msg1 = pd.read_csv(file_message_1, header=None, names=column_names)
    df_msg10 = pd.read_csv(file_message_10, header=None, names=column_names)
except FileNotFoundError as e:
    print(f"File not found: {e}")
    exit()

# Optional: Convert price to dollars and time to datetime (if desired)
# df_msg1["Price ($)"] = df_msg1["Price"] / 10000
# df_msg10["Price ($)"] = df_msg10["Price"] / 10000

# Optional: convert seconds since midnight to time format
def seconds_to_time(sec):
    from datetime import timedelta
    return timedelta(seconds=sec)

df_msg1["Time (hh:mm:ss)"] = df_msg1["Time"].apply(seconds_to_time)
df_msg10["Time (hh:mm:ss)"] = df_msg10["Time"].apply(seconds_to_time)

# Display basic info
print("📄 message_1.csv — Raw Events")
print(df_msg1.head())
print("\n📄 message_10.csv — Possibly Aggregated or Sampled Events")
print(df_msg10.head())

# Optional: Save preview to CSV for easier viewing in spreadsheet tools
df_msg1.head(1000).to_csv("preview_message_1.csv", index=False)
df_msg10.head(1000).to_csv("preview_message_10.csv", index=False)

print("\n✅ Preview CSVs saved as 'preview_message_1.csv' and 'preview_message_10.csv'")
