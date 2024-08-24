# This Generator creates the .csv file that simulates incoming orders for the order book.
# It contains "messages" that simulate incoming messages to the server with Add or Cancel orders.
# Add orders contain the order type with an OrderID, that shall be uniq and increasing.
# Cancel Order refers to the same ID.
import csv
import os
import random
from enum import Enum

#Custom variables for generation
NUM_OF_MESSAGES = 1000000 # 1 mil orders
OUTPUT_FILENAME = 'ExampleDataset.csv'
START_PRICE = 150

#Global variables
last_price = 100
first_iter = True
high_limit_mode = False
low_limit_mode= False

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
        price = last_price + random.choice([-3,-2,-1,0,1,2])  # bias towards reducing
        if price < 200: high_limit_mode = False
    elif low_limit_mode:
        price = last_price + random.choice([-2,-1,0,1,2,3])  # bias towards increasing
        if price > 200: low_limit_mode = False
    #else regular mode without bias to + or -
    else: price = last_price + random.choice([-2,-1,0,1,2])

    last_price = price
    quantity = random.randint(1, 100)

    return Order(ordertype, order_id, price, quantity)

def generate_random_orders(num_orders):

    orders = []
    order_ids_list = []  # store orderids from AddOrder so we can pick one in random to cancel

    for i in range(1, num_orders + 1):
        if order_ids_list and random.choice([True, False]): #todo adjust percentage
            # If we have orders in the list, pick a random one to cancel
            order_id_to_cancel = random.choice(order_ids_list)  # pick random id to cancel from list
            order_ids_list.remove(order_id_to_cancel)
            wrapped_order = CancelOrder(order_id_to_cancel)
        else: #If we have no order only choice is to create one
            order = generate_random_order(i)
            order_ids_list.append(order.order_id)
            wrapped_order = AddOrder(order)
        orders.append(wrapped_order)

    return orders

# Write the data to a CSV file
def write_order_messages_to_csv(order_messages, file_path):

    with open(file_path, mode='w', newline='') as file:
        writer = csv.writer(file)
        writer.writerow(['Message Type', 'Order ID', 'Order Type', 'Price', 'Quantity'])

        for order in order_messages:
            if isinstance(order, AddOrder):
                writer.writerow([
                    'AddOrder',                     #1. Msg type
                    order.order.order_id,           #2. Order id
                    order.order.ordertype.value,    #3. Order Type: Buy/sell/undef
                    order.order.price,              #4. Order Price
                    order.order.quantity            #5. Quantity
                ])
            elif isinstance(order, CancelOrder):    # This is not ideal, but decide not to focus on proper protocol for now
                writer.writerow([
                    'CancelOrder',                  #1. Msg type
                    order.order_id,                 #2. Order id
                    '',                             #3. Order Type: Buy/sell/undef
                    '',                             #4. Order Price
                    ''                              #5. Quantity
                ])

# Call generation, call write to .csv, save result to ../ExampleOrderDataset
def main(num_order_messages):

    # Determine the file path
    output_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', 'ExampleOrderDataset')
    os.makedirs(output_dir, exist_ok=True)
    file_path = os.path.join(output_dir, OUTPUT_FILENAME)

    # Generate Events
    order_messages = generate_random_orders(num_order_messages)

    # Write to CSV
    write_order_messages_to_csv(order_messages, file_path)
    print(f"CSV file created at: {file_path}")

if __name__ == '__main__':
    main(num_order_messages=NUM_OF_MESSAGES)