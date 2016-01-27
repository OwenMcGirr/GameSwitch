//
//  ViewController.m
//  GSKeys
//
//  Created by Owen McGirr on 26/01/2016.
//  Copyright © 2016 Owen McGirr. All rights reserved.
//

#import "ViewController.h"
#import <CoreBluetooth/CoreBluetooth.h>

@interface ViewController ()<CBCentralManagerDelegate, CBPeripheralDelegate>

@property (nonatomic) CBCentralManager *central;
@property (nonatomic) CBPeripheral *peripheral;

@property (nonatomic) CBUUID *uartServiceID;
@property (nonatomic) CBService *uartService;
@property (nonatomic) CBUUID *txCharacteristicID;
@property (nonatomic) CBCharacteristic *txCharacteristic;
@property (nonatomic) NSArray *desiredServiceIDs;
@property (nonatomic) NSArray *desiredCharacteristicIDs;

@end

@implementation ViewController

#pragma mark - view events

- (void)viewDidLoad {
    [super viewDidLoad];
    
    // connect
    self.central = [[CBCentralManager alloc] initWithDelegate:self queue:nil];
    self.uartServiceID = [CBUUID UUIDWithString:@"6e400001-b5a3-f393-e0a9-e50e24dcca9e"];
    self.desiredServiceIDs = @[self.uartServiceID];
    self.txCharacteristicID = [CBUUID UUIDWithString:@"6e400002-b5a3-f393-e0a9-e50e24dcca9e"];
    self.desiredCharacteristicIDs = @[self.txCharacteristicID];
}

- (void)viewWillAppear:(BOOL)animated {
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(keyboardWillShow:) name:UIKeyboardWillShowNotification object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(keyboardWillHide:) name:UIKeyboardWillHideNotification object:nil];
}

- (void)viewWillDisappear:(BOOL)animated {
    [[NSNotificationCenter defaultCenter] removeObserver:self name:UIKeyboardWillShowNotification object:nil];
    [[NSNotificationCenter defaultCenter] removeObserver:self name:UIKeyboardWillHideNotification object:nil];
}


#pragma mark - keyboard movements

- (void)keyboardWillShow:(NSNotification *)notification {
    CGSize keyboardSize = [[[notification userInfo] objectForKey:UIKeyboardFrameBeginUserInfoKey] CGRectValue].size;

    [UIView animateWithDuration:0.3 animations:^{
        CGRect f = self.view.frame;
        f.size.height = f.size.height - keyboardSize.height;
        self.view.frame = f;
    }];
    
    [self updateViewConstraints];
}

- (void)keyboardWillHide:(NSNotification *)notification {
    CGSize keyboardSize = [[[notification userInfo] objectForKey:UIKeyboardFrameBeginUserInfoKey] CGRectValue].size;

    [UIView animateWithDuration:0.3 animations:^{
        CGRect f = self.view.frame;
        f.size.height = f.size.height + keyboardSize.height;
        self.view.frame = f;
    }];
    
    [self updateViewConstraints];
}


#pragma mark - CBCentralManagerDelegate methods

- (void)centralManagerDidUpdateState:(CBCentralManager *)central {
    if (central.state == CBCentralManagerStatePoweredOn) {
        [central scanForPeripheralsWithServices:self.desiredServiceIDs options:nil];
    }
}

- (void)centralManager:(CBCentralManager *)central didDiscoverPeripheral:(CBPeripheral *)peripheral advertisementData:(NSDictionary *)advertisementData RSSI:(NSNumber *)RSSI {
    self.peripheral = peripheral;
    peripheral.delegate = self;
    [central connectPeripheral:peripheral options:nil];
}

- (void)centralManager:(CBCentralManager *)central didConnectPeripheral:(CBPeripheral *)peripheral {
    [peripheral discoverServices:self.desiredServiceIDs];
    NSLog(@"%@", peripheral.name);
}


#pragma mark - CBPeripheralDelegate methods

- (void)peripheral:(CBPeripheral *)peripheral didDiscoverServices:(NSError *)error {
    for (CBService *service in peripheral.services) {
        self.uartService = service;
        [peripheral discoverCharacteristics:self.desiredCharacteristicIDs forService:service];
    }
}

- (void)peripheral:(CBPeripheral *)peripheral didDiscoverCharacteristicsForService:(CBService *)service error:(NSError *)error {
    for (CBCharacteristic *characteristic in service.characteristics) {
        NSLog(@"%@", characteristic);
        self.txCharacteristic = characteristic;
        [peripheral setNotifyValue:YES forCharacteristic:characteristic];
    }
}


#pragma mark - IBActions

- (IBAction)sendUARTData:(id)sender {
    // create data
    NSData *data = [NSData dataWithBytes:[sendTextField text].UTF8String length:[sendTextField text].length];
    
    // send data
    [self.peripheral writeValue:data forCharacteristic:self.txCharacteristic type:CBCharacteristicWriteWithResponse];
    
    // add to activity field
    NSMutableString *activity = [NSMutableString stringWithString:[activityTextView text]];
    [activity appendString:@"\n"];
    [activity appendString:[sendTextField text]];
    [activityTextView setText:activity];
    
    // clear field
    [sendTextField setText:@""];
}

@end
