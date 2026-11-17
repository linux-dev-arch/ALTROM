code=~/altrom/examples/hello.bin
payload="../payload/payload.bin"
mtk_dump="../mtk_dump"
sudo $mtk_dump connect simple_da $payload 0x70009000
sudo $mtk_dump connect send_da $code 0 0
sudo $mtk_dump jump_da 0x70000000
