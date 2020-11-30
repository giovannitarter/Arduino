#!/usr/bin/python


import hid

vid = 0x16c0
pid = 0x05df

with hid.Device(vid, pid) as h:
    print(f'Device manufacturer: {h.manufacturer}')
    print(f'Product: {h.product}')
    print(f'Serial Number: {h.serial}')

    size = 128

    send_msg = [4, 5, 6, 7]
    text = bytes(send_msg + [0] * (size - len(send_msg)))
    print(text)
    h.send_feature_report(text)

    report_id = 0

    text = h.get_feature_report(report_id, size)
    print(text)
