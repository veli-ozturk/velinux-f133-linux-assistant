"""v5: v3 + zaman asiminda KISMI CIKTI (v4'un amaci). v4 KULLANILMAZ: heredoc ile yazilirken '\\002'/'\\003' kacislari
gercek STX/Ctrl+C baytlarina donustu; komut satirin ortasinda kesiliyor, kabuk kapanmamis tirnak bekliyordu
(11.10.2026 18:08; Ctrl+D ile kurtarildi, work/seri_ham_v1.py). Kullanim v3 ile ayni:
  python work/f133_seri_v5.py komut "echo; <komut>" [bekleme] --port=COM5"""
import sys
import time
import uuid
import f133_seri_v2 as old

STX, ETX = b'\x02', b'\x03'


class Kabuk(old.Kabuk):
    def run(self, cmd, wait=2.0, end_marker='__BITTI__'):
        del end_marker
        token = 'VE_DONE_' + uuid.uuid4().hex
        marker = STX + token.encode() + ETX
        # kabuga giden metinde YALNIZ yazdirilabilir karakterler: printf octal kacislari '\\002' ve '\\003' olarak
        wire = cmd + "; printf '" + chr(92) + "002" + token + chr(92) + "003" + chr(92) + "n'\n"
        assert all(32 <= ord(c) < 127 or c == '\n' for c in wire[len(cmd):]), 'kontrol karakteri gonderilmez'
        self.s.write(wire.encode())
        out = bytearray()
        deadline = time.monotonic() + wait + 30
        while time.monotonic() < deadline:
            out.extend(self.s.read(65536))
            if marker in out:
                data = bytes(out).split(marker, 1)[0]
                return '\n'.join(l for l in data.decode('utf-8', 'replace').splitlines() if token not in l)
        kismi = '\n'.join(l for l in bytes(out).decode('utf-8', 'replace').splitlines() if token not in l)
        raise TimeoutError('Seri komut bitis isareti gelmedi. KISMI CIKTI:\n' + kismi)


def main():
    if len(sys.argv) < 3 or sys.argv[1] != 'komut':
        raise SystemExit('Kullanim: f133_seri_v5.py komut <komut> [bekleme] --port=COM5')
    wait = float(sys.argv[3]) if len(sys.argv) > 3 and not sys.argv[3].startswith('--') else 2
    k = Kabuk(old.find_port())
    try:
        print(k.run(sys.argv[2], wait))
    except TimeoutError as e:
        print(e)
        raise SystemExit(124)
    finally:
        k.s.close()


if __name__ == '__main__':
    main()
