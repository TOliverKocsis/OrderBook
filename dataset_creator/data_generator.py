# This Generator creates the .csv file that simulates incoming orders for the order book.
# It contains "messages" that simulate incoming messages to the server with Add or Cancel orders.
# Add orders contain the order type with an OrderID, that shall be uniq and increasing.
# Cancel Order refers to the same ID.
import csv
import os
import random
from collections import deque
from enum import Enum

# Custom variables for generation
NUM_OF_MESSAGES = 100000
OUTPUT_FILENAME = 'example_dataset.csv'
START_PRICE = 150
PRICE_HISTORY_SIZE = 100  # Ask volume between prices uses a random choice from last PRICE_HISTORY_SIZE amount

# Global variables
last_price = 100
first_iter = True
high_limit_mode = False
low_limit_mode = False
latest_prices = deque(maxlen=PRICE_HISTORY_SIZE)  # Circular buffer to store the latest 100 ask prices generated


# Define the data structures
class OrderType(Enum):
    UNDEFINED = 'undefined'
    BUY = 'buy'
    SELL = 'sell'


class Order:
    def __init__(self, ordertype=OrderType.UNDEFINED, order_id=0, price=0, quantity=0):
        self.ordertype = ordertype
        self.order_id = order_id
        self.price = price
        self.quantity = quantity


class AddOrder:
    def __init__(self, order):
        self.order = order


class CancelOrder:
    def __init__(self, order_id):
        self.order_id = order_id


class GetBestBid:
    def __init__(self):
        pass


class GetAskVolumeBetweenPrices:
    def __init__(self, lower_price, upper_price):
        self.lower_price = lower_price
        self.upper_price = upper_price


# Generate random data
def generate_random_order(order_id):
    global first_iter
    global last_price
    if first_iter:
        last_price = START_PRICE
        first_iter = False
    ordertype = random.choice([OrderType.BUY, OrderType.SELL])

    global high_limit_mode
    global low_limit_mode
    # if price reaches 500, start a bias towards reduction until it gets back to 200
    if last_price >= 500: high_limit_mode = True
    # if price reaches 100, start a bias towards increasing, until it gets back to a 200
    if last_price <= 100: low_limit_mode = True

    if high_limit_mode:
        price = last_price + random.choice([-3, -2, -1, 0, 1, 2])  # bias towards reducing
        if price < 200: high_limit_mode = False
    elif low_limit_mode:
        price = last_price + random.choice([-2, -1, 0, 1, 2, 3])  # bias towards increasing
        if price > 200: low_limit_mode = False
    # else regular mode without bias to + or -
    else:
        price = last_price + random.choice([-2, -1, 0, 1, 2])

    last_price = price
    quantity = random.randint(1, 100)

    if ordertype == OrderType.SELL:  # as this is used for ask volume between prices, only save ask type price
        latest_prices.append(last_price)

    return Order(ordertype, order_id, price, quantity)


def generate_random_orders(num_orders):
    orders = []
    order_ids_list = []  # store orderids from AddOrder so we can pick one in random to cancel
    # store the previous 100 orders, for choice of GetAskVolumeBetweenPrices, in a circular buffer

    for i in range(1, num_orders + 1):
        # random.choices return  k (default=1) sized list with choices from population, biased by weights
        message_type = \
            random.choices(['AddOrder', 'CancelOrder', 'GetBestBid', 'GetAskVolumeBetweenPrices'],
                           [0.25, 0.15, 0.4, 0.2]
                           # AddOrder 25%
                           # CancelOrder 15%
                           # GetBestBid 40%
                           # GetAskVolume 20%
                           )[0]

        if message_type == 'AddOrder':
            order = generate_random_order(i)
            order_ids_list.append(order.order_id)
            wrapped_order = AddOrder(order)
        elif message_type == 'CancelOrder' and order_ids_list:
            order_id_to_cancel = random.choice(order_ids_list)
            order_ids_list.remove(order_id_to_cancel)
            wrapped_order = CancelOrder(order_id_to_cancel)
        elif message_type == 'GetBestBid':
            wrapped_order = GetBestBid()
        elif message_type == 'GetAskVolumeBetweenPrices' and len(latest_prices) >= 2:
            lower_price, upper_price = sorted(random.sample(latest_prices, 2))
            wrapped_order = GetAskVolumeBetweenPrices(lower_price, upper_price)
        else:
            # Default to AddOrder if no orders are available to cancel
            order = generate_random_order(i)
            order_ids_list.append(order.order_id)
            wrapped_order = AddOrder(order)
        orders.append(wrapped_order)
    return orders


# Write the data to a CSV file
def write_order_messages_to_csv(order_messages, file_path):
    with open(file_path, mode='w', newline='') as file:
        writer = csv.writer(file)
        writer.writerow(['Message Type', 'Order ID', 'Order Type', 'Price', 'Quantity', 'Lower Price', 'Upper Price'])

        for order in order_messages:
            if isinstance(order, AddOrder):
                writer.writerow(['AddOrder',  # 1. Msg type
                                 order.order.order_id,  # 2. Order ID
                                 order.order.ordertype.value,  # 3. Order Type: buy/sell/undefined
                                 order.order.price,  # 4. Order Price
                                 order.order.quantity,  # 5. Quantity
                                 '',  # 6. Lower Price
                                 ''  # 7. Upper Price
                                 ])
            elif isinstance(order, CancelOrder):
                writer.writerow(['CancelOrder',  # 1. Msg type
                                 order.order_id,  # 2. Order ID
                                 '',  # 3. Order Type
                                 '',  # 4. Order Price
                                 '',  # 5. Quantity
                                 '',  # 6. Lower Price
                                 ''  # 7. Upper Price
                                 ])
            elif isinstance(order, GetBestBid):
                writer.writerow(['GetBestBid',  # 1. Msg type
                                 '',  # 2. Order ID
                                 '',  # 3. Order Type
                                 '',  # 4. Order Price
                                 '',  # 5. Quantity
                                 '',  # 6. Lower Price
                                 ''  # 7. Upper Price
                                 ])
            elif isinstance(order, GetAskVolumeBetweenPrices):
                writer.writerow(['GetAskVolumeBetweenPrices',  # 1. Msg type
                                 '',  # 2. Order ID
                                 '',  # 3. Order Type
                                 '',  # 4. Order Price
                                 '',  # 5. Quantity
                                 order.lower_price,  # 6. Lower Price
                                 order.upper_price  # 7. Upper Price
                                 ])


# Call generation, call write to .csv, save result to ../example_order_dataset
def main(num_order_messages):
    # Determine the file path
    output_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'example_order_dataset')
    os.makedirs(output_dir, exist_ok=True)
    file_path = os.path.join(output_dir, OUTPUT_FILENAME)

    # Generate Events
    order_messages = generate_random_orders(num_order_messages)

    # Write to CSV
    write_order_messages_to_csv(order_messages, file_path)
    print(f"CSV file created at: {file_path}")


if __name__ == '__main__':
    main(num_order_messages=NUM_OF_MESSAGES)
