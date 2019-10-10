# Linear Wireless Sensor Network 환경에서 Slotted Aloha
**https://www.nsnam.org/**
NS-3 오픈소스를 활용하여 시뮬레이션을 구축하였으며, 수정한 일부분만 github에 올려두었습니다.

## Slotted Aloha 특징
* 데이터가 생기면 바로 전송한다
* 전송 실패가 존재한다. 재 전송에 많은 시간이 낭비된다
* Linear한 상황에서는 항상 전송실패가 나타나기에 좋지 못한 방법이다

## 주요 코드
* src/network/utils/simple-net-device.cc
* src/network/utils/simple-net-device.h
* src/network/model/lwsn-header.cc
* src/network/utils/simple-net-device.h
