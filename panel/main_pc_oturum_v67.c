/*
 * F133 masaustu panel arayuzu (LVGL 9.2, Linux fbdev + evdev, statik).
 *
 * 0.85 (25.10.2026, Claude): 0.84 (main_pc_oturum_v66.c) + canli kamera NV12 -> XRGB donusumu ve olcek G2D ile
 *     (ve_jpeg_panel_v3.h). Canli tamponlar G2D hedefi ION tamponlari (ek bellek yok); ilk G2D karesi CPU
 *     donusumuyle karsilastirilir, olmazsa CPU yolu. scripts/655.
 * 0.84 (25.10.2026, Claude): 0.83 (main_pc_oturum_v65.c) + canli kamera surekli cozme is parcacigi (dosya 5 ms'de
 *     bir yoklanir) + uc tampon; arayuz yalniz en son hazir kareyi gosterir (turda arama ~6/sn kare kaciriyordu). 653.
 * 0.83 (25.10.2026, Claude): 0.82 (main_pc_oturum_v64.c) + canli bilgi satiri saniyede bir (her turda ek cizim
 *     tetikliyordu; tek cekirdekte kamera-kayit kaynak hizini dusuruyordu). scripts/651.
 * 0.82 (25.10.2026, Claude): 0.81 (main_pc_oturum_v63.c) + canli kamera VE karesi XRGB8888 (ekran bicimi; LVGL donusumu
 *     kalkar), 5 sn istatistiginde cizim suresi ve tur sayilari. scripts/649.
 * 0.81 (25.10.2026, Claude): 0.80 (main_pc_oturum_v62.c) + OTONOM 49: canli kamera once Cedar VE donanim JPEG cozme
 *     (ve_jpeg_panel_v1.h; ilk kare yazilimla karsilastirilir, hata/wifi-ekran/panel-ve-kapali -> tjpgd). scripts/647.
 * 0.80 (24.10.2026, Claude): 0.79 (main_pc_oturum_v61.c) + OTONOM 150: 64 KB'i asan PC satirinin geri kalani da
 *     satir sonuna kadar atilir (yeni satir sanilmiyordu), cerceve_v4/tjpgd_v4 (IDCT carpimlari 64 bit). scripts/637.
 * 0.79 (24.10.2026, Claude): 0.78 (main_pc_oturum_v60.c) + denetim v3: 8765 CLOEXEC + bind yeniden deneme, PC saati
 *     toplam kayma siniri (saat_kayma_v1.h), PC satiri en cok 64 KB, panel.log 1 MB'ta kirpilir, BT kesme geri
 *     cekilmesi, tema sonrasi Soru yaz/Yeniden baslat, Wi-Fi cift is yok. scripts/622.
 * 0.76 (23.10.2026, Claude): 0.75 (main_pc_oturum_v57.c) + Wi-Fi sinyal/SSID wpa_supplicant kontrol soketinden
 *     (wpa_ctrl_v1.h; 10 sn'de bir iw popen = dakikada 12 fork kalkti), olmazsa iw. scripts/569.
 * 0.75 (23.10.2026, Claude): 0.74 (main_pc_oturum_v56.c) + HEY KOLE AC/KAPAT: Denetim Merkezi ust satirinda "Hey Kole"
 *     dugmesi, Ayarlar > KOLE anahtarina "Dinleme" etiketi, /data/asistan/kapali saniyede bir okunur (Telegram
 *     /heykole, sesle kapatma: kole-gemini v35), sesli {"heykole": false} yalniz kapatir. scripts/549.
 * 0.74 (22.10.2026, Claude): 0.73 (main_pc_oturum_v55.c) + TIME-LAPSE 1/3 SN VE OLAY TESPITI (kamera-kayit v9): kare
 *     araligi 1 sn, 3 sn, 10 sn, 30 sn, 1 dk, 5 dk; "Olay tespiti" anahtari (<kamera>/zaman-olay); durum satirinda olay
 *     kipi ve doluluk (zaman_durum_v1.h); sesli {"zaman_olay"}, {"zaman_aralik"} (kole-gemini v34). scripts/545.
 * 0.73 (22.10.2026, Claude): 0.72-v2 (main_pc_oturum_v54.c) + SESLI ISLEMLER: panel acildiktan sonraki ilk sesli islem
 *     artik yutulmuyor; sesle panel sesi ("ses", "sessiz", goreli "ses_rel"); Asistan "Gunun siber haberleri";
 *     PC Sistem surec karti iki sutun: en cok islemci | en cok bellek (pc_sistem_v2.h). scripts/534.
 * 0.72-v2 (22.10.2026, Claude): 0.72-v1 (main_pc_oturum_v53.c) + PC veri beslemeleri de etkinlik degil: pc_sistem
 *     (2 sn) ve pc_medya (30 sn) iletileri v1 cihazda dogrulanirken koruyucuyu yine kapatiyordu (ui_pasif_v2.h). scripts/529.
 * 0.72 (22.10.2026, Claude): 0.71 (main_pc_oturum_v52.c) + PC AJANI BAGLIYKEN EKRAN KORUYUCU: ajanin 2 sn'lik
 *     {"t":"ui","pc_status"} iletisi (ve gece parlakligi, gunluk istegi) artik etkinlik sayilmaz (ui_pasif_v1.h);
 *     onceden koruyucu hic otomatik acilmiyor, uzaktan acilan 2 sn'de kapaniyordu. scripts/527.
 * 0.71 (22.10.2026, Claude): 0.70 (main_pc_oturum_v51.c) + PC tam ekrandan cikista artik kalmaz: alici devri bitince
 *     tam ekran nesnesi yoksa tum ekran + katmanlar yeniden cizilir (bekleme sirasinda flush atlaniyordu). scripts/516.
 * 0.70 (22.10.2026, Claude): 0.69 (main_pc_oturum_v50.c) + Kamera arsivi (izleme): buyuk konum cubugu, altinda dugme yok
 *     (Daha eski/yeni sol sutunda), hiz 0.5x/1x/2x/4x/8x (4x/8x kare atlar), uzaktan "arsiv"/"arsiv_sec"/"arsiv_hiz".
 *     scripts/511.
 * 0.69 (22.10.2026, Claude): 0.68 (main_pc_oturum_v49.c) + Asistan once cihazdaki Gemini (PC bagliyken de; PC yedek),
 *     "Soru yaz" (Turkce klavye, serbest soru), "Soru: ..." satiri, uzaktan "asistan_sor" / "asistan_klavye". scripts/509.
 * 0.68 (22.10.2026, Claude): 0.67 (main_pc_oturum_v48.c) + sigmayan yazilar: Medya PC basligi iki satira kadar
 *     (uzunsa kalin 28 px), oynatma/ses dugmeleri kucuk (kirpilmiyor), Pano DURUM "CPU %6 · RAM %49", tema/koruyucu/
 *     Kole dugmelerinde metin payi, Baglantilar > Bilgisayar KOLE · PC servisi. scripts/507.
 * 0.67 (21.10.2026, Claude): 0.66 (main_pc_oturum_v47.c) + "PC Sistem" sekmesi (islemci/cekirdek, ekran karti, bellek,
 *     disk, ag, surecler; 3 dk gecmis grafikleri; pc_sistem_v1.h, PC: work/pc_sistem_v1.py). scripts/502.
 * 0.66 (21.10.2026, Claude): 0.65 (main_pc_oturum_v46.c) + Medya sayfasinda PC'de calan baslik / sanatci / uygulama /
 *     durum ({"t":"ui","pc_medya":{...}}, PC: work/pc_medya_v1.py). scripts/498.
 * 0.65 (21.10.2026, Claude): 0.64 (main_pc_oturum_v45.c) + PC Ekrani baglanma kipi Elle (varsayilan) / Otomatik
 *     (pc_session_v2.h, <veri>/pc-baglan-oto), "Baglan" / "Baglantiyi kes" dugmesi. scripts/494.
 * 0.64 (21.10.2026, Claude): 0.63 (main_pc_oturum_v44.c) + PC ikinci ekrani 180 derecede de aliciya devir (alici
 *     wifi-ekran v22 pad[10] bildirirse): /tmp/pc-dogrudan {nesil, izin, ters}. scripts/485.
 * 0.63 (21.10.2026, Claude): 0.62 (main_pc_oturum_v43.c) + kamera canli goruntu kucultulmus IDCT (cerceve_v3/tjpgd_v3:
 *     4:2:0 1/2-1/4 tam IDCT + ortalama yerine 4x4 / 2x2 IDCT). scripts/478.
 * 0.62 (21.10.2026, Claude): 0.61 (main_pc_oturum_v42.c) + yildiz koruyucusu: kirpma disindaki yildiz cizilmez, 60 sn'de
 *     bir kare maliyeti gunlugu, PANEL_YZ_TEST/PANEL_YZ_KIRPMA sinama kancalari. scripts/467.
 * 0.61 (21.10.2026, Claude): 0.60 (main_pc_oturum_v41.c) + hizli JPEG cozme (cerceve_v2/arsiv_v2/tjpgd_v2: 1/2-1/4'te
 *     YUV ortalamasi + tek renk donusumu, yonsuz karede esleme tablosu). scripts/451, 453.
 * 0.60 (21.10.2026, Claude): 0.59 (main_pc_oturum_v40.c) + surec tablosunda durum harfi, yuk/procs_blocked/iowait,
 *     D surecleri, MemAvailable. scripts/441.
 * 0.59 (21.10.2026, Claude): 0.58 (main_pc_oturum_v39.c) + bosta CPU: pc_image_tick PC yokken 100 ms. scripts/432.
 * 0.58 (21.10.2026, Claude): 0.57 (main_pc_oturum_v38.c) + /data/hoparlor-dogrulandi, pc_ajan_bagli() (state ile),
 *     PC Ekrani'nda Ses: panel/PC dugmesi (ses_yonu), PC yokken br_pc=-1. scripts/423.
 * 0.57 (21.10.2026, Claude): 0.56 (main_pc_oturum_v37.c) + Denetim Merkezi Kamera 3 mod (Kapali/Kayit/Time-lapse),
 *     {"t":"ui","bildirim":..} -> Telegram kuyrugu, PC Ekrani'nda "PC'den ekran iste". scripts/417.
 * 0.56 (21.10.2026, Claude): 0.55 (main_pc_oturum_v36.c) + KAMERA ARSIVI (kayit / time-lapse / fotograf izleme,
 *     arsiv_v1.h), canli goruntu dogrudan ekran boyutunda cozme + istatistik, uzaktan surec tablosu (gunluk surec) ve
 *     gunluk aramasi, BT bekcisi 5 sn. scripts/413.
 * 0.55 (20.10.2026, Claude): 0.54 (main_pc_oturum_v35.c) + Bluetooth Acik/Kapali ayari (kapaliyken kesilir, yeniden
 *     baglanirsa tekrar kesilir), Time-lapse (kamera-kayit zaman-acik/zaman-aralik) Ayarlar > Gizlilik'te, canli goruntu
 *     Akici/Net + kare/sn. scripts/409.
 * 0.54 (20.10.2026, Claude): 0.53 (main_pc_oturum_v34.c) + Ayarlar > Ekran ikiye bolundu (Ekran / Koruyucu; Ekran
 *     tasiyordu ve kaydirilamiyordu), tum Ayarlar bolumleri kaydirilabilir, Ayarlar > Gizlilik'te Canli goruntu. scripts/405.
 * 0.53 (20.10.2026, Claude): 0.52 (main_pc_oturum_v33.c) + Sistem: Yeniden baslat (onayli) / Asistani yenile, Gizlilik:
 *     Canli goruntu (kamera-kayit /tmp/kamera-son.jpg, yalniz bu ekranda), Bluetooth baglan/kes (MCU D4 AT#CA+CC /
 *     AT#CD+CB), Ayarlar kayma duzeltmeleri (sol menu simge sutunu, Kole satirlari, koruyucu, hizmetler). scripts/401.
 * 0.52 (20.10.2026, Claude): 0.51 (main_pc_oturum_v32.c) + PC'siz asistan (PC ajani yoksa istem -> cihazdaki
 *     kole-gemini --metin), varsayilan cihaz istemleri, zamanlayici zili cihaz hoparlorunden (ses_kart_v2.h),
 *     uzaktan {"t":"ui","zamanlayici_sn":N}. scripts/388.
 * 0.51 (20.10.2026, Claude): 0.50 (main_pc_oturum_v31.c) + ekran 180° acikken PC goruntusu aliciya (wifi-ekran
 *     dogrudan cizim, ROT_90 sabit) DEVREDILMEZ; panel kendi yolunu kullanir. scripts/374.
 * 0.50 (20.10.2026, Claude): 0.49 (main_pc_oturum_v30.c) + Hey Kole "Anladigim: <soru>" satiri (kole-asistan v11
 *     "soru" alani), olay okuma tamponu 2048. scripts/365.
 * 0.49 (20.10.2026, Claude): 0.48 (main_pc_oturum_v29.c) + Hey Kole ses/altyazi senkronu (/tmp/tts-sure, /tmp/tts-caliyor)
 *     ve salt okunur gunluk tanilamasi {"t":"ui","gunluk":..}. scripts/353.
 * 0.48 (20.10.2026, Claude): 0.47 (main_pc_oturum_v28.c) + Pano namaz/imsak satiri kaldirildi, eski "ses yolu yok"/A3
 *     metinleri guncellendi, gece kipi secili koruyucuyu (Robot goz) korur. scripts/343.
 * 0.47 (20.10.2026, Claude): 0.46 (main_pc_oturum_v27.c) + EKRAN 180 DERECE (varsayilan acik; G2D ROT_270, CPU, PC
 *     dogrudan cizim, dokunmatik), Ayarlar anahtari + uzaktan "ekran_ters"; acilis animasyonu 1.6x (~4.9 sn). scripts/339.
 * 0.46 (19.10.2026, Claude): 0.45 (main_pc_oturum_v26.c) + Gizlilik > BUGUNUN KAYITLARI (kayit_ozet_v1.h, SD salt
 *     okuma: parca/sure/boyut, araliklar, en guclu hareketler, zaman atlamali). Ayrinti scripts/325.
 * 0.45 (19.10.2026, Claude): 0.44 (main_pc_oturum_v25.c) + TASARIM 1: kayan sekme gostergesi, kademeli kart girisi,
 *     dokunma halkasi, gradyanli kartlar, 7 gunluk hava/takvim seridi (U2). Ayrinti scripts/318.
 * 0.44 (19.10.2026, Claude): 0.43 (main_pc_oturum_v24.c) + 4 yeni koruyucu: Kelime saati, Dunya saatleri, Yildiz alani,
 *     Aurora (panel_ayar_v4.h koruyucu_tur 7..10). Ayrinti scripts/314.
 * 0.43 (19.10.2026, Claude): 0.42 (main_pc_oturum_v23.c) + YENI MARKA (gorunen her yer KOLE robot usak, logo_assets_v2.c)
 *     + ACILIS ANIMASYONU (VeLinux A cizilir -> KOLE robotu belirir, ~3 sn, dokunma atlar; PANEL_ACILIS=0) + KAPALI.
 *     Ayrinti scripts/310.
 * 0.42 (19.10.2026, Claude): 0.41 (main_pc_oturum_v22.c) + Ayarlar > Sistem HIZMETLER sutunu (8 hizmet /proc cmdline
 *     onekiyle, bot/saglik.log/autorun/cekirdek ozeti; 5 sn de bir, yalniz bolum gorunurken). Ayrinti scripts/299.
 * 0.41 (19.10.2026, Claude): 0.40 (main_pc_oturum_v21.c) + OTOMATIK GECE KIPI 23-07: rahatsiz etme (kaydedilmeden),
 *     parlaklik <= %30, koruyucu Karartma, "Gece kipi" hapi. Ayarlar > Ekran anahtari (/data/panel-gece-kapali),
 *     uzaktan {"t":"ui","gece":bool}, test PANEL_GECE_ZORLA. Ayrinti scripts/292.
 * 0.40 (19.10.2026, Claude): 0.39 (main_pc_oturum_v20.c) + varsayilan tema Kirmizi & Siyah (cihazda bir kez gecis,
 *     /data/panel-tema-040) + 4 yeni ekran koruyucu: Analog saat, Kayan logo, Bilgi panosu, Karartma
 *     (panel_ayar_v3.h koruyucu_tur 3..6). Ayarlar > Ekran: 7 tur + Onizle. Ayrinti scripts/287.
 * 0.39 (19.10.2026, Claude): 0.38 (main_pc_oturum_v19.c) + SES KARTI (ses_kart_v1.h): ses kaydiricisi/sessiz F133
 *     codec'ini surer (Headphone volume 0..7, sessizde cikis anahtarlari 0 + /tmp/ses-sessiz), "Test tonu" dugmesi
 *     ve uzaktan {"t":"ui","ses_test":true}. MCU'ya gonderim YOK. Ayrinti scripts/283.
 * 0.38 (15.10.2026, Claude): 0.37 (main_pc_oturum_v18.c) + G2D ILE DONDURME: LVGL tamponlari ION'da, flush G2D ROT_90
 *     (CPU karo dondurme 35-50 ms -> ~5 ms tam ekran). Cekirdek/flash degismez. Ayrinti scripts/243.
 * 0.37 (15.10.2026, Claude): 0.36 (main_pc_oturum_v17.c) + DIJITAL CERCEVE: Ayarlar > Ekran "Fotoğraf çerçevesi";
 *     koruyucu yerine /mnt/sd/fotograf JPEG'leri 20 sn'de bir (cerceve_v1.h, tjpgd). Ayrinti scripts/227.
 * 0.36 (15.10.2026, Claude): 0.35 (main_pc_oturum_v16.c) + sesli kamera: fotograf (Telegram), bekci, zaman atlamali
 *     (kamera-kayit v3 bayraklari; kalkan aciksa reddedilir); DURUM bekci/zaman. Ayrinti scripts/225.
 * 0.35 (15.10.2026, Claude): 0.34 (main_pc_oturum_v15.c) + hatirlatici/alarm (Kole yuzu + Telegram kuyrugu), Pano namaz
 *     vakti ve haber basliklari, DURUM siradaki hatirlatici. Ayrinti scripts/218.
 * 0.34 (15.10.2026, Claude): 0.33 (main_pc_oturum_v14.c) + karo (16x16) dondurmeli aktarim, gecis cizim/aktarim olcumu,
 *     sesli goreli parlaklik (br_rel). Ayrinti scripts/208.
 * 0.33 (15.10.2026, Claude): 0.32 (main_pc_oturum_v13.c) + yumusak sekme gecisi (yalniz acilan sayfa kayar, olculur),
 *     Kole yuzu goz acilisi, Ayarlar > Ekran "Animasyon" anahtari; DURUM satirlari, kur virgulu. Ayrinti scripts/206.
 * 0.32 (15.10.2026, Claude): 0.31 (main_pc_oturum_v12.c) + sesli panel islemleri (/tmp/asistan-eylem.json), PC'siz Pano
 *     verisi (/tmp/panel-veri.json), yeni Pano (saat | hava | piyasa + durum). Ayrinti scripts/202.
 * 0.31 (15.10.2026, Claude): 0.30 (main_pc_oturum_v11.c) + Gemini akisi: "dinliyor"/"dusunuyor" olaylari yuzu acik tutar
 *     (ust yazi, dusunurken gozler yukari). Ayrinti scripts/193.
 * 0.30 (15.10.2026, Claude): 0.29 (main_pc_oturum_v10.c) + Ayarlar > KOLE: agiz (5), goz (4), goz hareketi (3), renk (7),
 *     karakter (3), onizle; Hey Kole/ogret/altyazi buraya tasindi. /data/asistan/yuz.conf. Ayrinti scripts/182.
 * 0.29 (14.10.2026, Claude): 0.28 (main_pc_oturum_v9.c) + HEY KOLE: konusan yuz (agiz animasyonu + acilip kapanir
 *     altyazi), Ayarlar > Ses: asistan anahtari + "Sesimi ogret" + altyazi anahtari. Ayrinti scripts/176.
 * 0.28 (14.10.2026, Claude): 0.27 (main_pc_oturum_v8.c) + MT 10 yuva: Goodix yuva = yonga dokunus kimligi (0..9);
 *     0.27 yalniz yuva 0/1'i izledigi icin iki parmak gorulmuyordu. Hareket etkin ilk iki yuvayla. Ayrinti scripts/165.
 * 0.27 (14.10.2026, Claude): 0.26 (main_pc_oturum_v7.c) + PC tam ekraninda IKI PARMAK (sikistir/ac = yakinlastir,
 *     kaydir = teker/yatay teker; alici v17 + PC gonderici v27), MT yuvalari ayri izlenir (imlec ziplamaz), ust
 *     kenar dokunusu PC'ye tiklama, cubuk yalniz kenardan asagi kaydirinca. Ayrinti scripts/161.
 * 0.26 (14.10.2026, Claude): 0.25 (main_pc_oturum_v6.c) + PC ekrani istegi cihazdaki aliciyi baslatir/durdurur:
 *     pc_session.want -> /tmp/ikinci-ekran-acik (autorun v12+ wifi-ekran gozeticisi 2 sn'de bir bakar). Istek bitince
 *     (sayfadan cikis / Baglantiyi kes) bayrak silinir, alici durur, bellek Tailscale/kameraya kalir.
 * 0.25 (14.10.2026, Claude): 0.24 (main_pc_oturum_v5.c) + KAMERA KAYDI PANELDEN PAROLASIZ BASLATILIR (kullanıcı: "panel
 *     uzerinden istersem kamerayi acabileyim sifre girmeden"). Dokunmatik cihazin basinda olmayi gerektirir; uzaktan
 *     acma KOLE Merkez parolasiyla kalir. Gizlilik dugmesi ve Denetim Merkezi Kamera kutucugu: kayit kapaliyken tek
 *     dokunusla /mnt/sd/kamera/kayit-acik (2 kare/sn, 1 dk parca, 6 saat dongu; bayrak SD'de, yeniden acilista surer),
 *     aciksa eskisi gibi onayli "Kamerayi kapat". Kalkan aciksa baslatmaz. Canli izleme (/tmp/kamera-aktif) panelden
 *     acilmaz.
 * 0.24 (13.10.2026, Claude, cihazda HENUZ DENENMEDI): 0.23 (main_pc_oturum_v4.c, aynen korunuyor) + ROBOT GOZ koruyucu.
 *   - Ekran koruyucu turu (Ayarlar > Ekran, /data/panel.conf koruyucu_tur, panel_ayar_v2.h): Saat (0.23 koruyucusu) /
 *     Robot goz (varsayilan) / Karisik (goz + altta kucuk saat-tarih-hava satiri).
 *   - Robot goz (robot_goz_v1.h saf durum makinesi, tohumlu RNG): iki ifadeli goz; bos, kirp, bak, gezin, suphe, mutlu,
 *     saskin, uykulu, uyu (gece 23-07 baskin, zzz), kizgin/kalp (nadir), tara. Tepkiler: /tmp/kamera-aktif -> kirmizi
 *     "izleniyor" + "KAMERA ACIK" hapi; /tmp/mik-aktif -> dinleme + "MIKROFON" hapi; dokunma -> o yone bakip mutlu
 *     kirpar, ~0.6 sn sonra koruyucu kapanir (parlaklik dokununca hemen geri gelir; ikinci dokunus hemen kapatir);
 *     PC bildirimi -> saskin (Rahatsiz etme acikken yok).
 *   - Cizim: tek nesnenin DRAW_MAIN olayinda LVGL ilkelleri (yuvarlak dikdortgen, daire, yay, ucgen, birkac harf);
 *     resim yok. Yalniz gozlerin (eski U yeni) kutulari gecersiz kilinir. Zamanlayici gecis sirasinda 40 ms (<= 25 fps)
 *     ya da 80 ms, durgunken sonraki olaya kadar (<= 1 sn) uyur. panel.log'a 30 sn'de bir "robot goz:" olcum satiri
 *     (guncelleme/sn, kare basina alan, cizim suresi ort/azami).
 *   - Denetim Merkezi: baslik satirinda "Robot goz" kisayolu; Koruyucu kutucugu alt yazisi secili turu gosterir.
 *   - Uzaktan (yeni): {"t":"ui","ss":"saat|robot|karisik"} (o turla hemen ac, kaydetmez), {"t":"ui","ss_tur":"..."|0..2}
 *     (kaydeder), {"t":"ui","rg":"<davranis>"} (yalniz bu anahtar varsa koruyucuyu KAPATMAZ; robot goz acikken o
 *     davranisi baslatir: bos/kirp/bak/gezin/suphe/mutlu/saskin/uykulu/uyu/kizgin/kalp/tara/izleniyor/dinliyor/uyan).
 *   - Test kancalari (yalniz PC/WSL): PANEL_RG_TOHUM=<sayi> (belirlenimci davranis), PANEL_RG_GECE=0|1 (gece zorla).
 *   - MCU'ya / ALSA'ya hicbir sey gonderilmez; yeni kutuphane yok.
 * 0.23 (13.10.2026, Claude, cihazda HENUZ DENENMEDI): 0.22 (main_pc_oturum_v3.c) + Denetim Merkezi + yeni sayfalar.
 *   - DENETIM MERKEZI: ust seridi asagi cekince (ya da seride dokununca) ustten kayan perde: Wi-Fi, Bluetooth
 *     (yalniz durum), Kamera (acik ise onayli kapatma = 0.22 mantigi), Gizlilik kalkani, Parlaklik ve Ses kaydiricilari,
 *     Rahatsiz etme, Gece modu (otomatik parlaklik), Tema, Panel kilidi, Koruyucuyu simdi ac, Otomatik koruyucu.
 *     Yukari kaydirma / perde disina dokunma kapatir. Perde LAZY kurulur, kapaninca silinir (bellek); ekranin son
 *     cocugu oldugu icin altindaki arayuz (opak alanda) cizilmez; animasyon <= 220 ms, yalniz y konumu.
 *   - SES DUZEYI: cihazda guvenli ses yolu YOK (codec/BT hoparlor yolu cozulmedi). Deger /data/panel.conf'a
 *     yazilir, ekranda "ses yolu henuz yok" yazar. MCU'ya / ALSA'ya HICBIR SEY gonderilmez.
 *   - Sekmeler 10: Pano, Medya, Kisayollar, Asistan, Zamanlayici, Notlar, Monitor, PC Ekrani, Gizlilik, Ayarlar.
 *     Uzaktan {"t":"ui","tab":0..6} ESKI (0.22) anlamini korur (KOLE Merkez/ajan uyumu), yeni: {"t":"ui","sayfa":"medya"}.
 *   - Ayarlar (eski Sistem): bolumler Ekran / Ses / Baglantilar / Gizlilik / Sistem; eski islevlerin hepsi (otomatik/
 *     manuel parlaklik, koruyucu, tema, Wi-Fi kurulumu, BT gostergesi, surum/bellek) + koruyucu suresi, rahatsiz etme.
 *   - Medya: PC medya tuslari (0.22'deki makro olaylari: playpause/prev/next/voldown/mute/volup, PC ajani isler) +
 *     telefon BT muzik DURUMU (yalniz /tmp/bt-durum.json okunur; telefon kontrolu "yakinda", MCU'ya gonderim YOK).
 *   - Notlar: yapilacaklar listesi /data/notlar.txt (en fazla 16), Turkce klavye.
 *   - Monitor: islemci/bellek grafigi (60 sn), sicaklik, calisma suresi, yuk, panel bellegi, Wi-Fi hizi, depolama.
 *   - Panel kilidi (yanlis dokunmaya karsi, guvenlik degil): kaydirarak acilir; kamera/mikrofon hapi kilitte de gorunur.
 *   - Bildirim gecmisi (son 8) ve Rahatsiz etme: PC'den gelen bildirimler gosterilmez, gecmise yazilir.
 *   - Uzaktan (yeni): "sayfa", "dm":1|0, "kilit":true|false, "dnd":true|false, "ses":0..100, "ayar":"ekran|ses|
 *     baglanti|gizlilik|sistem", "not_ekle":"...", "not_klavye":true, "koruyucu_dk":N.
 *   - Test kancalari (yalniz PC/WSL provasi; cihazda verilmez): PANEL_VERI_DIZIN=<dizin> (/data yerine),
 *     PANEL_PROC_KOK=<dizin> (/proc, /sys okumalarinin onune eklenir; ornek olcum dosyalari),
 *     PANEL_TEST_DOKUNMA=<fifo> (dokunmatik yerine "<0|1> x y" satirlari; surukleme hareketlerinin PC provasi).
 *   - panel.log'a: "denetim merkezi: acildi/kapandi", "kilit: acik/kapali" (cihazda tani icin).
 * 0.22 (13.10.2026, Claude, cihazda HENUZ DENENMEDI): 0.21 (main_pc_oturum_v2.c) + arayuz cilasi ve yeni Gizlilik sekmesi.
 *   - Ust serit: sag kumede hap (chip) gostergeler: gizlilik (kirmizi KAMERA / turuncu MIKROFON / yesil KALKAN),
 *     PC (renkli nokta), tarih-saat, Wi-Fi 4 cubuk sinyal + IP. Seridin altinda ince ayrac.
 *   - Kartlar: 1 px kenarlik, kart basliklari vurgu renkli simge + aralikli kucuk harf; metrik cubuklari tema renginde.
 *   - Sekme cubugu: secili sekme vurgu rengi + ustte 4 px cizgi.
 *   - Bildirim (toast) 180 ms belirerek acilir/kapanir (kucuk alan, ucuz).
 *   - Ekran koruyucu: saat + tarih + hava + gizlilik uyarisi; dakikada bir 1.2 sn yumusak kayma (iz kalmasin).
 *   - Yeni sekme 6 "Gizlilik": kamera-kayit durumu (/tmp/kamera-kayit.json, YALNIZ OKUMA: canli/kayit/fps/parca/MB/
 *     SD bos), mikrofon durumu (/tmp/mik-aktif, yalniz gosterge), "Kamerayi kapat" (onayli; yalniz KAPATIR:
 *     kole-kopru v6 "KAYIT 0" + "KAMERA 0" ile ayni iki bayragi siler, kayitli dosyalara dokunmaz) ve
 *     "Gizlilik kalkani" (RAM'de, varsayilan kapali; aciksa kamera bayraklari her saniye silinir, /tmp/gizlilik-kalkani).
 *   - Uzaktan: {"t":"ui","tab":0..6}, {"t":"ui","ss":1} (ekran koruyucuyu hemen ac),
 *     {"t":"ui","kamera_kapat_sor":true} (yalniz onay penceresi; kapatmak icin ekranda "Kapat"a dokunmak gerekir).
 *   - Test kancasi: PANEL_FB_DOSYA=<dosya> ortam degiskeni verilirse /dev/fb0 yerine 1600x600 XRGB8888 duz dosya
 *     kullanilir (yalniz PC/WSL'de ekran goruntusu provasi; cihazda degisken verilmez).
 * 0.19 (11.10.2026, Claude): devir dikey fb'de de (flash cekirdegi, alici v13 mod 2: G2D mixer + dondurme).
 *   Devir geri alinirken alicinin onayi (pad[0]==nesil, pad[1]==0) beklenir (en cok 400 ms): bu arada panel
 *   ortak bellekteki eski goruntuyu cizmez/flush etmez (0.18 canli: "arada kare kare renk bozulmasi").
 * 0.18 (11.10.2026, Claude): ALICIYA DEVIR. Dogrudan cizim kosullari saglaninca /tmp/pc-dogrudan dosyasina
 *   {nesil, izin=1} yazilir; alici v12 basligin pad[0..1]'ine ayni nesli ve 1 yazinca (sahip) panel PC
 *   dikdortgenlerini cizmez, LVGL flush fb'ye yazmaz (alici G2D ile ciziyor). Cubuk/bildirim/cikis aninda izin=0;
 *   alici fb->ortak bellek esitler ve tam ekran dikdortgeni yayinlar. Alici v11 ise onay gelmez: 0.17-v3 yolu.
 *   PANEL_G2D_ALICI=0 devri kapatir.
 * 0.17-v3: kirli dikdortgenler 16x16 karo haritasinda birlestirilir (v2 canli: cizim basina 2.4 Mpiksel, ust uste
 *   binen alici dikdortgenleri; 172 ms/cizim), satir parcalari 64 bit yazmayla iki piksel birden yazilir.
 *   JSON'a dc_cv_us (yalniz donusum) eklendi.
 * 0.17-v2: ust katman denetimi yalniz GORUNUR nesnelere bakar (v1: gizli toast/ekran koruyucu/Wi-Fi penceresi
 *   hep katmanda oldugu icin dogrudan yol hic calismadi, canli olcum dc_n=0).
 * 0.17 DOGRUDAN CIZIM (11.10.2026, Claude): PC tam ekrandayken (ust cubuk/ust katman bos) alicinin kirli
 *   dikdortgenleri LVGL'siz, ortak bellekten (RGB565) dogrudan fb0'a (XRGB8888) donusturulur, tum kirli
 *   alanlar icin TEK FBIOPAN_DISPLAY (G2D dondurme). 0.16 olcumu: LVGL yenilemesi 80-195 ms/5-11 fps darbogazdi.
 *   PANEL_DOGRUDAN=0 ortam degiskeni eski LVGL yolunu zorlar. Cubuk gorunurken LVGL yolu kullanilir.
 * G2D prova: fb0 1600x600, rotate=1; son LVGL flush sonrasi FBIOPAN_DISPLAY.
 * Eski 600x1600 framebuffer icin yazilim 90 derece yolu da korunur.
 * Dokunmatik: GT913 ham 0..1023 x 0..599 (DTS'te iki eksen ters cevrili) -> 1600x600'e olceklenir.
 * PC baglantisi: panel TCP 8765'te dinler, PC ajani (work\panel_ajan_v3.py) baglanir. Her satir bir JSON:
 *   PC -> panel : {"t":"state","time":...,"pc":{...},"weather":{...},"fx":{...},"events":[...],
 *                  "macros":[{"id","label"}],"prompts":[{"id","label"}],"assistant":{"text","busy"},"toast":"..."}
 *   panel -> PC : {"t":"macro","id":"..."}  {"t":"ask","id":"..."}  {"t":"hello","ver":"..."}
 *   (Baglanti PC'den panele kuruldugu icin Windows guvenlik duvari gelen baglanti izni istemez.)
 */
#define _GNU_SOURCE
#include "lvgl/lvgl.h"
#include "cJSON.h"
#include "bt_status.h"
#include "kamera_durum_v1.h"
#include "pc_shm_v1.h"
#include "pc_session_v2.h"	/* 0.65: baglanma kipi (v1: sekmeye girince baglanir) */
#include "panel_ayar_v4.h"	/* 0.44 (v3: 0.40) */
#include "robot_goz_v1.h"
#include "notlar_v1.h"
#include "sistem_olcum_v1.h"
#include "bt_medya_v1.h"
#include "ve_jpeg_panel_v3.h"	/* 0.85: v3 G2D; 0.82 v2 XRGB8888; 0.81 v1 */
#include "cerceve_v4.h"	/* 0.80: v4 (tjpgd_v4: 64 bit IDCT); 0.63: v3 (tjpgd_v3.c icerir); 0.61 v2; 0.37 v1 */
#include "arsiv_v2.h"	/* 0.61: v2 (cf_tablo_birak); 0.56: kamera arsivi, kare tarama, olcekli cozme */
#include <linux/dma-buf.h>	/* 0.38 */
#include <stdbool.h>
#include "g2d_driver_tina.h"
#include "ses_kart_v2.h"	/* 0.39; 0.52: v2 (zamanlayici zili) */
#include "kayit_ozet_v1.h"	/* 0.46 */
#include "pc_sistem_v2.h"	/* 0.73: v2 (ilk_ram); 0.67: PC Sistem verisi */
#include "ui_pasif_v2.h"	/* 0.72-v2: etkinlik sayilmayan ui iletisi (v1 + pc_sistem, pc_medya) */
#include "zaman_durum_v1.h"	/* 0.74: time-lapse durum/doluluk metinleri */
#include "wpa_ctrl_v1.h"	/* 0.76: wpa_supplicant kontrol soketi (iw popen yerine) */
#include "izinli_ip_v1.h"	/* 0.77: 8765 baglanti izni (denetim v2 bulgu 6) */
#include "saat_kayma_v1.h"	/* 0.79: PC saati toplam kayma siniri (denetim v3 A11) */
static struct pcs_veri pcs;	/* 0.67 */
static uint32_t pcs_t;
static lv_obj_t *pcs_l_durum, *pcs_l_cpu, *pcs_l_cpu_d, *pcs_ch_cpu, *pcs_ch_cek, *pcs_l_gpu, *pcs_l_gpu_d, *pcs_ch_gpu,
	*pcs_l_ram, *pcs_l_ram_d, *pcs_ch_ram, *pcs_l_disk, *pcs_l_disk_d, *pcs_ch_disk, *pcs_sur_bar[PCS_SUR], *pcs_sur_l[PCS_SUR],
	*pcs_l_ag, *pcs_l_ag_d, *pcs_ch_ag, *pcs_ilk_l[PCS_ILK][4];	/* 0.73: islemci ad/% | bellek ad/MB */
static lv_chart_series_t *pcs_s_cpu, *pcs_s_cek, *pcs_s_gpu, *pcs_s_ram, *pcs_s_disk, *pcs_s_in, *pcs_s_out;
static void ek_temizle(void);	/* 0.40 ek koruyucular (asagida) */
static void aa_temizle(void);	/* 0.43 acilis animasyonu (asagida) */
#include <dirent.h>
#include <sys/resource.h>
#include <sys/syscall.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <linux/input.h>
#include <linux/kd.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <ctype.h>
#include <sys/reboot.h>	/* 0.53 */
#include <sys/statvfs.h>
#include <sys/sysinfo.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#define PANEL_VER "0.85-ve-kamera-g2d-v1 (25.10.2026)"
#define APP_ADI "KÖLE"	/* 0.43: urun adi; VeLinux yalniz acilista */
LV_IMAGE_DECLARE(amblem_40);
LV_IMAGE_DECLARE(logo_tam);
LV_IMAGE_DECLARE(kole_yazi_ust);	/* 0.43 */
LV_IMAGE_DECLARE(velinux_ve);
LV_IMAGE_DECLARE(velinux_linux);
#define W 1600
#define H 600
#define PORT 8765
#define MAX_BTN 12

/* 0.23 sekme sirasi (ekranda soldan saga). Uzaktan "tab" 0..6 eski (0.22) anlamindadir: eski_sekme[] esler. */
enum { S_PANO, S_MEDYA, S_KISAYOL, S_ASISTAN, S_ZAMAN, S_NOTLAR, S_MONITOR, S_PCSIS, S_PC, S_GIZLILIK, S_AYARLAR, S_N };	/* 0.67 S_PCSIS */
static const uint8_t eski_sekme[7] = {S_PANO, S_KISAYOL, S_ASISTAN, S_ZAMAN, S_AYARLAR, S_PC, S_GIZLILIK};
static const char *const sekme_adi[S_N] = {"pano", "medya", "kisayollar", "asistan", "zamanlayici", "notlar", "monitor",
					   "pcsistem", "pc", "gizlilik", "ayarlar"};
static int kilitli;	/* 0.23 panel kilidi (asagida) */
static void dm_git(int ac, int anim);	/* 0.23 Denetim Merkezi (asagida) */

/* ------------------------------------------------------------------ ekran */
static uint8_t *fbm;
static uint32_t fb_line, fb_xres, fb_yres;
static int fb_rot, fb_fd = -1, fb_hw_pan;
static struct fb_var_screeninfo fb_var;
static uint64_t pc_pan_us, pc_pan_calls, pc_pan_errors;
static uint64_t pc_dc_us, pc_dc_n, pc_dc_px, pc_dc_cv_us;	/* 0.17 dogrudan cizim: sure (donusum+pan), pan sayisi, piksel */
static int pc_dc_izin = 1;
static int pc_dg_fd = -1, pc_dg_izin = -1, pc_dg_acik = 1, pc_rv_sahip, pc_dg_ters = -1;	/* 0.18 aliciya devir; 0.64 yon */
static uint32_t pc_dg_gen, pc_rv_rects, pc_dg_t0, pc_dg_bekleme;
static int pc_dg_bekle;
static uint32_t pc_dg_atlanan;	/* 0.71: devir/bekleme sirasinda atlanan flush (tanilama) */
static int pc_view_active;
static uint64_t pc_flush_us, pc_flush_calls;
/* 0.14: bir yenileme turunun (REFR_START -> son flush) suresi, PC goruntusu acikken */
static struct timespec pc_refr_t0;
static uint64_t pc_render_sum;
static uint32_t pc_render_n, pc_render_max;
/* 0.33 sekme gecis animasyonu: acik/kapali (/data/panel-gecis-kapali) ve olcum */
static int gc_acik = 1, gc_olcum;
static uint32_t gc_kare, gc_t0;
static uint64_t gc_aktarim_us, gc_cizim_us;	/* 0.34 olcum */
static int ekran_ters = 1;	/* 0.47: dikey fb 180 derece (main: /data/panel-ekran-duz yoksa 1) */
/* 0.45: tum sekme degisimleri sekme_ayarla() uzerinden (kayan gosterge + kademeli kart girisi; asagida) */
static void sekme_ayarla(lv_obj_t *tv, uint32_t i, lv_anim_enable_t a);
#define lv_tabview_set_active(tv, i, a) sekme_ayarla(tv, i, a)
static void sk_gec(uint32_t i, int anim);
static void dk_baslat(int32_t x, int32_t y);	/* 0.45 dokunma halkasi */
static void hf_kur(lv_obj_t *kart);	/* 0.45 haftalik serit (asagida) */
static void hf_veri(cJSON *j);
static void gc_refr_cb(lv_event_t *e)
{
	(void)e;
	if (gc_olcum)
		gc_kare++;
}

static void refr_start_cb(lv_event_t *e)
{
	(void)e;
	clock_gettime(CLOCK_MONOTONIC, &pc_refr_t0);
}

static uint64_t elapsed_us(const struct timespec *a, const struct timespec *b)
{
	return (uint64_t)((b->tv_sec-a->tv_sec)*1000000 + (b->tv_nsec-a->tv_nsec)/1000);
}

/* 0.23 test kancalari: PANEL_VERI_DIZIN (/data yerine) ve PANEL_PROC_KOK (/proc, /sys okumalarinin onune). */
static char veri_dizin[128] = "/data", proc_kok[128] = "";

static void veri_yol(char *o, size_t n, const char *ad)
{
	snprintf(o, n, "%s/%s", veri_dizin, ad);
}

static void proc_yol(char *o, size_t n, const char *yol)
{
	snprintf(o, n, "%s%s", proc_kok, yol);
}

/* ilk n-1 bayt (NUL sonlu); donus: okunan bayt, hata -1 */
static int dosya_oku(const char *yol, char *buf, size_t n)
{
	int fd = open(yol, O_RDONLY | O_CLOEXEC);
	if (fd < 0 || !n) {
		if (n)
			buf[0] = 0;
		if (fd >= 0)
			close(fd);
		return -1;
	}
	size_t k = 0;
	while (k + 1 < n) {
		ssize_t r = read(fd, buf + k, n - 1 - k);
		if (r <= 0)
			break;
		k += (size_t)r;
	}
	close(fd);
	buf[k] = 0;
	return (int)k;
}

/* yol.yeni'ye yaz, rename (yarim dosya kalmasin); donus 0 tamam */
static int dosya_yaz_atomik(const char *yol, const char *veri, size_t n)
{
	char tmp[200];
	snprintf(tmp, sizeof tmp, "%s.yeni", yol);
	FILE *f = fopen(tmp, "w");
	if (!f)
		return -1;
	int ok = fwrite(veri, 1, n, f) == n;
	if (fclose(f))
		ok = 0;
	if (!ok || rename(tmp, yol)) {
		unlink(tmp);
		return -1;
	}
	return 0;
}

/* 0.22 test kancasi: PANEL_FB_DOSYA verilirse duz dosyaya (1600x600 XRGB8888, yatay, pan yok) cizilir. */
static int fb_init_dosya(const char *yol)
{
	size_t n = (size_t)W * H * 4;
	int fd = open(yol, O_RDWR | O_CREAT | O_CLOEXEC, 0644);

	if (fd < 0 || ftruncate(fd, (off_t)n)) {
		perror("PANEL_FB_DOSYA");
		return -1;
	}
	fbm = mmap(NULL, n, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (fbm == MAP_FAILED)
		return -1;
	memset(fbm, 0, n);
	fb_fd = fd;
	fb_line = W * 4;
	fb_xres = W;
	fb_yres = H;
	fb_rot = 0;
	fb_hw_pan = 0;
	const char *dk = getenv("PANEL_FB_DIKEY");	/* 0.47 host: dikey fb, CPU dondurme yolu */
	if (dk && dk[0] == '1') {
		fb_line = H * 4;
		fb_xres = H;
		fb_yres = W;
		fb_rot = 1;
	}
	printf("fb: test dosyasi %s (%ux%u)\n", yol, fb_xres, fb_yres);
	return 0;
}

static int fb_init(void)
{
	struct fb_var_screeninfo v;
	struct fb_fix_screeninfo f;
	const char *test_dosya = getenv("PANEL_FB_DOSYA");
	if (test_dosya && test_dosya[0])
		return fb_init_dosya(test_dosya);
	int fd = open("/dev/fb0", O_RDWR | O_CLOEXEC);	/* 0.79: system() ile baslayan surece gecmesin */

	if (fd < 0 || ioctl(fd, FBIOGET_VSCREENINFO, &v) || ioctl(fd, FBIOGET_FSCREENINFO, &f)) {
		perror("fb0");
		return -1;
	}
	if (v.bits_per_pixel != 32) {
		fprintf(stderr, "fb0 %d bpp desteklenmiyor\n", v.bits_per_pixel);
		return -1;
	}
	fb_line = f.line_length;
	fb_xres = v.xres;
	fb_yres = v.yres;
	fb_rot = fb_yres > fb_xres;
	if ((fb_rot ? fb_yres : fb_xres) != W || (fb_rot ? fb_xres : fb_yres) != H ||
	    fb_line < fb_xres * 4u || f.smem_len < (size_t)fb_line * fb_yres ||
	    v.xoffset || v.yoffset) {
		fprintf(stderr, "fb0 geometri/uzunluk/ofset desteklenmiyor\n");
		close(fd);
		return -1;
	}
	fb_fd = fd;
	fb_var = v;
	fb_hw_pan = !fb_rot && v.rotate == 1;
	fbm = mmap(NULL, f.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (fbm == MAP_FAILED)
		return -1;
	memset(fbm, 0, (size_t)fb_line * fb_yres);
	/* fbcon'un imleci ve yazilari arayuzun ustune cizmesin */
	int tty = open("/dev/tty1", O_RDWR);
	if (tty >= 0) {
		ioctl(tty, KDSETMODE, KD_GRAPHICS);
		close(tty);
	}
	printf("fb0 %ux%u satir %u, yazilim-rot=%d rotate=%u G2D-pan=%d\n", fb_xres, fb_yres, fb_line, fb_rot, v.rotate, fb_hw_pan);
	fflush(stdout);
	return 0;
}

/* ------------------------------------------------------------ 0.38 G2D dondurme */
struct pg_ion_ayir { uint64_t len; uint32_t heap_id_mask, flags, fd, unused; };
struct pg_ion_yigin { char name[32]; uint32_t type, heap_id, r0, r1, r2; };
struct pg_ion_sorgu { uint32_t cnt, r0; uint64_t heaps; uint32_t r1, r2; };
struct pg_dma_param { int fd; unsigned int phy_addr; };
#define PG_ION_ALLOC _IOWR('I', 0, struct pg_ion_ayir)
#define PG_ION_HEAP_QUERY _IOWR('I', 8, struct pg_ion_sorgu)
#define PG_VE_ENGINE_REQ 0x206
#define PG_VE_ENGINE_REL 0x207
#define PG_VE_MAP_DMA_BUF 0x504
static int pg_acik, pg_g2d = -1, pg_ve = -1, pg_ion = -1, pg_fd[2] = { -1, -1 }, pg_hata;
static uint8_t *pg_buf[2];
static uint32_t pg_adr[2], pg_fb_phys;
static size_t pg_boy;
static unsigned long long pg_say, pg_us;

static int pg_esitle(int fd, int bas)
{
	struct dma_buf_sync s = { (bas ? DMA_BUF_SYNC_START : DMA_BUF_SYNC_END) | DMA_BUF_SYNC_WRITE };
	return ioctl(fd, DMA_BUF_IOCTL_SYNC, &s);
}

static void pg_kapat(void)
{
	/* tamponlar LVGL'de kalabilir: mmap ve fd'ler acik birakilir, yalniz G2D yolu kapanir */
	pg_acik = 0;
}

/* 0: G2D hazir, tamponlar pg_buf[0/1]; -1: eski yol */
static int pg_hazirla(size_t bsz)
{
	const char *e = getenv("PANEL_G2D");
	if (!fb_rot || (e && !strcmp(e, "0"))) {
		printf("G2D dondurme: kapali (%s)\n", fb_rot ? "PANEL_G2D=0" : "fb yatay");
		return -1;
	}
	struct fb_fix_screeninfo f;
	if (ioctl(fb_fd, FBIOGET_FSCREENINFO, &f) || !f.smem_start || f.line_length != fb_xres * 4) {
		puts("G2D dondurme: kapali (fb bilgisi)");
		return -1;
	}
	pg_fb_phys = (uint32_t)f.smem_start;
	pg_g2d = open("/dev/g2d", O_RDWR | O_CLOEXEC);
	pg_ion = open("/dev/ion", O_RDWR | O_CLOEXEC);
	pg_ve = open("/dev/cedar_dev", O_RDWR | O_CLOEXEC);
	const char *neden = "aygit yok";
	if (pg_g2d < 0 || pg_ion < 0 || pg_ve < 0)
		goto hata;
	struct pg_ion_yigin hd[16];
	memset(hd, 0, sizeof hd);
	struct pg_ion_sorgu q = { .cnt = 16, .heaps = (uintptr_t)hd };
	uint32_t mask = 0;
	neden = "ION yigini";
	if (ioctl(pg_ion, PG_ION_HEAP_QUERY, &q))
		goto hata;
	for (unsigned i = 0; i < q.cnt && i < 16; i++)
		if (hd[i].type == 0)	/* sistem yigini: CMA'ya dokunmaz (alici ayrilmis yigini kullaniyor) */
			mask = 1u << hd[i].heap_id;
	if (!mask)
		goto hata;
	neden = "ION ayirma";
	for (int i = 0; i < 2; i++) {
		struct pg_ion_ayir a = { .len = bsz, .heap_id_mask = mask, .flags = 1 /* ION_FLAG_CACHED */ };
		if (ioctl(pg_ion, PG_ION_ALLOC, &a))
			goto hata;
		pg_fd[i] = (int)a.fd;
		pg_buf[i] = mmap(NULL, bsz, PROT_READ | PROT_WRITE, MAP_SHARED, pg_fd[i], 0);
		if (pg_buf[i] == MAP_FAILED) {
			pg_buf[i] = NULL;
			goto hata;
		}
	}
	/* VE saati acikken esle (kapaliyken ilk IOMMU eslemesi cekirdekte takiliyordu, alici v13 notu) */
	neden = "VE ENGINE_REQ";
	if (ioctl(pg_ve, PG_VE_ENGINE_REQ, 0))
		goto hata;
	int mh = 0;
	for (int i = 0; i < 2 && !mh; i++) {
		struct pg_dma_param p = { .fd = pg_fd[i] };
		mh = ioctl(pg_ve, PG_VE_MAP_DMA_BUF, &p);
		pg_adr[i] = p.phy_addr;
	}
	ioctl(pg_ve, PG_VE_ENGINE_REL, 0);
	neden = "VE MAP_DMA_BUF";
	if (mh)
		goto hata;
	for (int i = 0; i < 2; i++)
		pg_esitle(pg_fd[i], 1);
	pg_boy = bsz;
	pg_acik = 1;
	printf("G2D dondurme: acik (ION sistem, 2 x %zu B, IOMMU 0x%08x 0x%08x, fb 0x%08x)\n", bsz, pg_adr[0], pg_adr[1], pg_fb_phys);
	return 0;
hata:
	printf("G2D dondurme: kapali (%s: %s)\n", neden, strerror(errno));
	for (int i = 0; i < 2; i++) {
		if (pg_buf[i])
			munmap(pg_buf[i], bsz);
		pg_buf[i] = NULL;
		if (pg_fd[i] >= 0)
			close(pg_fd[i]);
		pg_fd[i] = -1;
	}
	if (pg_ve >= 0)
		close(pg_ve);	/* kapanis eslemeleri birakir */
	if (pg_ion >= 0)
		close(pg_ion);
	if (pg_g2d >= 0)
		close(pg_g2d);
	pg_ve = pg_ion = pg_g2d = -1;
	return -1;
}

/* 0: alan G2D ile fb'ye yazildi; -1: cagiran CPU ile cizer */
static int pg_dondur(const uint8_t *px, const lv_area_t *a)
{
	int i = px == pg_buf[0] ? 0 : px == pg_buf[1] ? 1 : -1;
	int32_t w = lv_area_get_width(a), h = lv_area_get_height(a);
	if (i < 0 || a->x1 < 0 || a->y1 < 0 || (uint32_t)a->x2 >= fb_yres || (uint32_t)a->y2 >= fb_xres || (size_t)w * h * 4 > pg_boy)
		return -1;
	struct timespec t0, t1;
	clock_gettime(CLOCK_MONOTONIC, &t0);
	pg_esitle(pg_fd[i], 0);	/* onbellek -> DRAM (G2D okuyacak) */
	g2d_blt_h b;
	memset(&b, 0, sizeof b);
	b.flag_h = ekran_ters ? G2D_ROT_270 : G2D_ROT_90;	/* 0.47 */
	b.src_image_h.format = G2D_FORMAT_ARGB8888;
	b.src_image_h.laddr[0] = pg_adr[i];
	b.src_image_h.use_phy_addr = 1;
	b.src_image_h.width = (unsigned)w;
	b.src_image_h.height = (unsigned)h;
	b.src_image_h.clip_rect = (g2d_rect){ 0, 0, (unsigned)w, (unsigned)h };
	b.src_image_h.alpha = 0xff;
	b.dst_image_h.format = G2D_FORMAT_ARGB8888;
	b.dst_image_h.laddr[0] = pg_fb_phys;
	b.dst_image_h.use_phy_addr = 1;
	b.dst_image_h.width = fb_xres;
	b.dst_image_h.height = fb_yres;
	b.dst_image_h.clip_rect = ekran_ters ? (g2d_rect){ a->y1, (int)fb_yres - a->x1 - w, (unsigned)h, (unsigned)w }
					     : (g2d_rect){ (int)fb_xres - a->y1 - h, a->x1, (unsigned)h, (unsigned)w };
	b.dst_image_h.alpha = 0xff;
	int r = ioctl(pg_g2d, G2D_CMD_BITBLT_H, &b);
	pg_esitle(pg_fd[i], 1);
	clock_gettime(CLOCK_MONOTONIC, &t1);
	if (r) {
		if (++pg_hata <= 3)
			perror("G2D dondurme");
		if (pg_hata >= 3) {
			puts("G2D dondurme: 3 hata, CPU yoluna donuldu");
			pg_kapat();
		}
		return -1;
	}
	pg_say++;
	pg_us += elapsed_us(&t0, &t1);
	return 0;
}

static void flush_cb(lv_display_t *d, const lv_area_t *a, uint8_t *px)
{
	int32_t w = lv_area_get_width(a);
	struct timespec start, end;
	int last = lv_display_flush_is_last(d);
	if (pc_rv_sahip || pc_dg_bekle) {	/* 0.18: fb'yi alici G2D ile ciziyor; devir bitince tam ekran yeniden cizilir */
		pc_dg_atlanan += !pc_rv_sahip;	/* 0.71 */
		lv_display_flush_ready(d);
		return;
	}
	if (pc_view_active) clock_gettime(CLOCK_MONOTONIC, &start);

	struct timespec g0;	/* 0.34 gecis olcumu */
	if (gc_olcum)
		clock_gettime(CLOCK_MONOTONIC, &g0);
	if (fb_rot && pg_acik && !pg_dondur(px, a)) {
		/* 0.38: G2D dondurme birimi yazdi */
	} else if (fb_rot) {	/* 0.34: 16x16 karolar (0.33: sutun sutun, kaynak w*4 bayt adimla okunuyordu) */
		for (int32_t y0 = a->y1; y0 <= a->y2; y0 += 16) {
			int32_t y1 = y0 + 15 > a->y2 ? a->y2 : y0 + 15;
			for (int32_t x0 = a->x1; x0 <= a->x2; x0 += 16) {
				int32_t x1 = x0 + 15 > a->x2 ? a->x2 : x0 + 15;
				for (int32_t x = x0; x <= x1; x++) {
					if (x < 0 || (uint32_t)x >= fb_yres)
						continue;
					const uint32_t *src = (const uint32_t *)px + (size_t)(y0 - a->y1) * w + (x - a->x1);
					if (ekran_ters) {	/* 0.47: (x,y) -> satir yres-1-x, sutun y */
						uint32_t *dst = (uint32_t *)(fbm + (size_t)(fb_yres - 1 - (uint32_t)x) * fb_line);
						for (int32_t y = y0; y <= y1; y++, src += w)
							dst[y] = *src | 0xFF000000u;
						continue;
					}
					uint32_t *dst = (uint32_t *)(fbm + (size_t)x * fb_line) + (fb_xres - 1);
					for (int32_t y = y0; y <= y1; y++, src += w)
						dst[-y] = *src | 0xFF000000u;
				}
			}
		}
	} else {
		for (int32_t y = a->y1; y <= a->y2; y++) {
			uint32_t *dst = (uint32_t *)(fbm + (size_t)y * fb_line) + a->x1;
			const uint32_t *src = (const uint32_t *)px + (size_t)(y - a->y1) * w;
			for (int32_t x = 0; x < w; x++)
				dst[x] = src[x] | 0xFF000000u;
		}
	}
	/* Pan only after all partial buffers of this refresh are copied. G2D rotates
	 * the entire mapped source; calling once per partial wastes bandwidth/time.
	 * panel is launched in the background so a driver stall leaves COM5 usable. */
	if (last && fb_hw_pan) {
		struct timespec p0, p1;
		clock_gettime(CLOCK_MONOTONIC, &p0);
		int r = ioctl(fb_fd, FBIOPAN_DISPLAY, &fb_var);
		clock_gettime(CLOCK_MONOTONIC, &p1);
		if (pc_view_active) {
			pc_pan_calls++;
			pc_pan_us += elapsed_us(&p0, &p1);
		}
		if (r < 0) {
			pc_pan_errors++;
			if (pc_pan_errors <= 2) perror("G2D FBIOPAN_DISPLAY");
		}
	}
	if (gc_olcum) {
		struct timespec g1;
		clock_gettime(CLOCK_MONOTONIC, &g1);
		gc_aktarim_us += elapsed_us(&g0, &g1);
		if (last)
			gc_cizim_us += elapsed_us(&pc_refr_t0, &g1);
	}
	lv_display_flush_ready(d);
	if (pc_view_active) {
		clock_gettime(CLOCK_MONOTONIC, &end);
		pc_flush_us += (uint64_t)((end.tv_sec-start.tv_sec)*1000000 + (end.tv_nsec-start.tv_nsec)/1000);
		pc_flush_calls++;
		if (last) {
			uint32_t us = (uint32_t)((end.tv_sec - pc_refr_t0.tv_sec) * 1000000 + (end.tv_nsec - pc_refr_t0.tv_nsec) / 1000);
			pc_render_sum += us;
			pc_render_n++;
			if (us > pc_render_max)
				pc_render_max = us;
		}
	}
}

static uint32_t tick_ms(void)
{
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return (uint32_t)(t.tv_sec * 1000 + t.tv_nsec / 1000000);
}

/* ------------------------------------------------------------- dokunmatik */
static int ev_fd = -1, ev_x, ev_y, ev_down, ev_xmax = 1023, ev_ymax = 599;
/* 0.27: MT B tipi: ilk iki yuva (parmak) ayri; ev_x/ev_y yalniz ABS_X/ABS_Y'den (cekirdek isaretci oykunmesi, en eski
 * parmak). ABS_X hic gelmezse eski davranis: yuva 0'in MT konumu. */
#define MT_YUVA_N 10	/* 0.28: Goodix GT9xx en cok 10 nokta; yuva numarasi yonganin dokunus kimligi */
static int mt_yuva, mt_id[MT_YUVA_N] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1}, mt_x[MT_YUVA_N], mt_y[MT_YUVA_N], ev_abs_xy;
static void pc_jest_isle(void);	/* 0.27 (asagida, PC tam ekran iki parmak) */

/* 0.23 test kancasi: PANEL_TEST_DOKUNMA=<fifo> verilirse dokunmatik yerine FIFO'dan "<0|1> x y" satirlari okunur
 * (ekran koordinati). Yalniz PC/WSL provasinda surukleme/dokunma hareketlerini denemek icin; cihazda verilmez. */
static int test_dok_fd = -1;
static char test_dok_buf[256];
static size_t test_dok_n;

static void test_dokunma_oku(void)
{
	for (;;) {
		ssize_t r = read(test_dok_fd, test_dok_buf + test_dok_n, sizeof test_dok_buf - 1 - test_dok_n);
		if (r <= 0)
			break;
		test_dok_n += (size_t)r;
		test_dok_buf[test_dok_n] = 0;
		char *nl;
		while ((nl = memchr(test_dok_buf, '\n', test_dok_n))) {
			*nl = 0;
			int d, x, y;
			if (sscanf(test_dok_buf, "%d %d %d", &d, &x, &y) == 3 && x >= 0 && x < W && y >= 0 && y < H) {
				ev_down = !!d;
				ev_x = x;
				ev_y = y;
			}
			test_dok_n -= (size_t)(nl + 1 - test_dok_buf);
			memmove(test_dok_buf, nl + 1, test_dok_n);
		}
		if (test_dok_n >= sizeof test_dok_buf - 1)
			test_dok_n = 0;	/* asiri uzun satir: at */
	}
}

static void touch_init(void)
{
	char p[32], name[64];
	const char *td = getenv("PANEL_TEST_DOKUNMA");

	if (td && td[0]) {
		test_dok_fd = open(td, O_RDONLY | O_NONBLOCK | O_CLOEXEC);
		ev_xmax = W - 1;
		ev_ymax = H - 1;
		printf("dokunmatik: test FIFO %s (%s)\n", td, test_dok_fd >= 0 ? "acik" : "acilamadi");
		return;
	}

	for (int i = 0; i < 8; i++) {
		snprintf(p, sizeof p, "/dev/input/event%d", i);
		int fd = open(p, O_RDONLY | O_NONBLOCK | O_CLOEXEC);	/* 0.79 */
		if (fd < 0)
			continue;
		name[0] = 0;
		ioctl(fd, EVIOCGNAME(sizeof name), name);
		if (strstr(name, "Goodix") || strstr(name, "Touch")) {
			struct input_absinfo ai;
			if (!ioctl(fd, EVIOCGABS(ABS_X), &ai) && ai.maximum > 0)
				ev_xmax = ai.maximum;
			if (!ioctl(fd, EVIOCGABS(ABS_Y), &ai) && ai.maximum > 0)
				ev_ymax = ai.maximum;
			ev_fd = fd;
			printf("dokunmatik %s (%s) %dx%d\n", p, name, ev_xmax, ev_ymax);
			return;
		}
		close(fd);
	}
	printf("dokunmatik bulunamadi\n");
}

static void touch_read(lv_indev_t *i, lv_indev_data_t *data)
{
	struct input_event ev;

	(void)i;
	if (test_dok_fd >= 0)
		test_dokunma_oku();
	while (ev_fd >= 0 && read(ev_fd, &ev, sizeof ev) == sizeof ev) {
		if (ev.type == EV_ABS) {
			int y2 = mt_yuva >= 0 && mt_yuva < MT_YUVA_N;
			switch (ev.code) {
			case ABS_X:
				ev_x = ekran_ters ? ev_xmax - ev.value : ev.value;	/* 0.47 */
				ev_abs_xy = 1;
				break;
			case ABS_Y:
				ev_y = ekran_ters ? ev_ymax - ev.value : ev.value;
				ev_abs_xy = 1;
				break;
			case ABS_MT_SLOT:
				mt_yuva = ev.value;
				break;
			case ABS_MT_TRACKING_ID:
				if (y2)
					mt_id[mt_yuva] = ev.value;
				break;
			case ABS_MT_POSITION_X: {
				int v = ekran_ters ? ev_xmax - ev.value : ev.value;	/* 0.47 */
				if (y2)
					mt_x[mt_yuva] = v;
				if (!ev_abs_xy && mt_yuva == 0)
					ev_x = v;
			}
				break;
			case ABS_MT_POSITION_Y: {
				int v = ekran_ters ? ev_ymax - ev.value : ev.value;
				if (y2)
					mt_y[mt_yuva] = v;
				if (!ev_abs_xy && mt_yuva == 0)
					ev_y = v;
			}
				break;
			default:
				break;
			}
		} else if (ev.type == EV_KEY && ev.code == BTN_TOUCH) {
			ev_down = ev.value;
			if (!ev_down)
				for (int k = 0; k < MT_YUVA_N; k++)
					mt_id[k] = -1;	/* tum parmaklar kalkti (kayip SYN'e karsi) */
		}
	}
	pc_jest_isle();	/* 0.27 */
	data->point.x = (int32_t)((int64_t)ev_x * (W - 1) / ev_xmax);
	data->point.y = (int32_t)((int64_t)ev_y * (H - 1) / ev_ymax);
	data->state = ev_down ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
	static int dk_onceki;	/* 0.45: basma kenari -> dokunma halkasi */
	if (ev_down && !dk_onceki)
		dk_baslat(data->point.x, data->point.y);
	dk_onceki = ev_down;
}

/* ------------------------------------------------------------- parlaklik */
/* Tina disp2: DISP_LCD_SET_BRIGHTNESS 0x102, GET 0x103; args {ekran, deger} */
/* Current board/flash PWM is inverse: increasing the driver level dims the
 * physical panel (kullanıcı confirmed). Keep UI, PC and screensaver levels logical:
 * larger means brighter. Mirror only at this device boundary, within 3..255.
 * Recheck this mapping if a future kernel/DT fixes the PWM polarity. */
static int disp_brightness(int set)
{
	int logical = set < 3 ? 3 : set > 255 ? 255 : set;
	unsigned long a[4] = {0, (unsigned long)(258 - logical), 0, 0};
	int fd = open("/dev/disp", O_RDWR | O_CLOEXEC), r = -1;	/* 0.79 */

	if (fd < 0)
		return -1;
	r = ioctl(fd, set >= 0 ? 0x102 : 0x103, a);
	close(fd);
	if (set < 0 && r >= 0) {
		r = 258 - r;
		r = r < 3 ? 3 : r > 255 ? 255 : r;
	}
	return r;
}

/* ------------------------------------------------------------------- ag */
static pthread_mutex_t mu = PTHREAD_MUTEX_INITIALIZER;
static char *pending;		/* PC'den gelen son durum satiri */
static char *ui_pending[8];	/* {"t":"ui",...} komutlari (durum satiri ezmesin diye ayri kuyruk) */
static int ui_n;
static char outq[16][192];
static int outn;
static volatile int pc_conn;
static volatile time_t pc_last;	/* monoton saniye (saat ayari atlamasindan etkilenmez) */
static time_t mono_s(void)
{
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return t.tv_sec;
}
static char pc_peer[48] = "-";

static void send_event(const char *json)
{
	pthread_mutex_lock(&mu);
	if (outn < 16)
		snprintf(outq[outn++], sizeof outq[0], "%s\n", json);
	pthread_mutex_unlock(&mu);
}

/* 0.49: dosyanin son 10 satiri (son 4 KB icinden), her satir ayri olay (kuyruk satiri 192 bayt) */
static void gunluk_satir(const char *ad, const char *t)	/* 0.56 */
{
	cJSON *o = cJSON_CreateObject();
	cJSON_AddStringToObject(o, "t", "gunluk");
	cJSON_AddStringToObject(o, "ad", ad);
	cJSON_AddStringToObject(o, "s", t);
	char *m = cJSON_PrintUnformatted(o);
	if (m && strlen(m) < 190)
		send_event(m);
	free(m);
	cJSON_Delete(o);
}

static void gunluk_gonder(const char *ad, const char *yol, const char *ara)	/* 0.56: ara = son 64 KB'ta suzgec */
{
	static char b[65537];
	FILE *f = fopen(yol, "r");
	if (!f)
		return;
	long pencere = ara && ara[0] ? 65536 : 4096;
	long boy = fseek(f, 0, SEEK_END) ? 0 : ftell(f);
	long bas = boy > pencere ? boy - pencere : 0;
	fseek(f, bas, SEEK_SET);
	size_t n = fread(b, 1, (size_t)pencere, f);
	fclose(f);
	b[n] = 0;
	static char *sat[2048];
	int k = 0;
	for (char *p = strtok(b, "\n"); p && k < 2048; p = strtok(NULL, "\n"))
		if (!ara || !ara[0] || strstr(p, ara))
			sat[k++] = p;
	for (int i = k > 10 ? k - 10 : 0; i < k; i++) {
		if (strlen(sat[i]) > 140)
			sat[i][140] = 0;
		gunluk_satir(ad, sat[i]);
	}
}

/* 0.56 uzaktan "top": {"t":"ui","gunluk":"surec"} -> /proc/<pid>/stat utime+stime 1 sn arayla; en cok CPU alan 9 surec +
 * ozet (cihaz yuku 4-5 idi, kaynagi bilinmiyordu). Salt okuma. */
#define SR_AZAMI 200
struct sr_s {
	int pid;
	unsigned long long t;
	long rss;
	char ad[24];
	char durum;	/* 0.60: R S D Z T I */
};
static struct sr_s sr_a[SR_AZAMI], sr_b[SR_AZAMI];
static int sr_an;
static unsigned long long sr_cpu_a, sr_iow_a;	/* 0.60: iowait */
static lv_timer_t *sr_tmr;

static unsigned long long sr_iow_son;	/* 0.60: son okumadaki iowait (5. alan) */
static unsigned long long sr_cpu_toplam(void)
{
	char y[160], b[512];
	proc_yol(y, sizeof y, "/proc/stat");
	if (dosya_oku(y, b, sizeof b) <= 0)
		return 0;
	unsigned long long v[8] = {0};
	if (sscanf(b, "cpu %llu %llu %llu %llu %llu %llu %llu %llu", &v[0], &v[1], &v[2], &v[3], &v[4], &v[5], &v[6], &v[7]) < 4)
		return 0;
	unsigned long long t = 0;
	for (int i = 0; i < 8; i++)
		t += v[i];
	sr_iow_son = v[4];
	return t;
}

/* 0.60: /proc/stat "<ad> N" satiri (procs_running / procs_blocked); yoksa -1 */
static long sr_stat_sayi(const char *ad)
{
	char y[160];
	static char b[4096];
	proc_yol(y, sizeof y, "/proc/stat");
	if (dosya_oku(y, b, sizeof b) <= 0)
		return -1;
	const char *p = strstr(b, ad);
	return p ? atol(p + strlen(ad)) : -1;
}

static int sr_oku(struct sr_s *o, int azami)
{
	char y[160];
	proc_yol(y, sizeof y, "/proc");
	DIR *dd = opendir(y);
	struct dirent *e;
	int n = 0;
	while (dd && (e = readdir(dd)) && n < azami) {
		if (e->d_name[0] < '1' || e->d_name[0] > '9')
			continue;
		char p[300], b[640];
		snprintf(p, sizeof p, "%.160s/%.20s/stat", y, e->d_name);
		if (dosya_oku(p, b, sizeof b) <= 0)
			continue;
		char *a = strchr(b, '('), *z = strrchr(b, ')');
		if (!a || !z || z < a)
			continue;
		struct sr_s *s = &o[n];
		s->pid = atoi(b);
		int al = (int)(z - a - 1);
		if (al > 23)
			al = 23;
		memcpy(s->ad, a + 1, (size_t)al);
		s->ad[al] = 0;
		unsigned long long ut = 0, st = 0;
		long rss = 0;
		int alan = 3;	/* z+2: state = 3. alan; utime 14, stime 15, rss 24 */
		char *kal = NULL;
		s->durum = '?';
		for (char *tok = strtok_r(z + 2, " ", &kal); tok; tok = strtok_r(NULL, " ", &kal), alan++) {
			if (alan == 3)
				s->durum = tok[0];	/* 0.60 */
			else if (alan == 14)
				ut = strtoull(tok, NULL, 10);
			else if (alan == 15)
				st = strtoull(tok, NULL, 10);
			else if (alan == 24) {
				rss = atol(tok);
				break;
			}
		}
		s->t = ut + st;
		s->rss = rss * 4;
		n++;
	}
	if (dd)
		closedir(dd);
	return n;
}

static void sr_bitir_cb(lv_timer_t *t)
{
	(void)t;
	sr_tmr = NULL;
	unsigned long long c = sr_cpu_toplam(), dc = c > sr_cpu_a ? c - sr_cpu_a : 1;
	unsigned long long diow = sr_iow_son > sr_iow_a ? sr_iow_son - sr_iow_a : 0;	/* 0.60 */
	int bn = sr_oku(sr_b, SR_AZAMI);
	static unsigned long long dt[SR_AZAMI];
	unsigned long long top = 0;
	for (int i = 0; i < bn; i++) {
		dt[i] = 0;
		for (int j = 0; j < sr_an; j++)
			if (sr_a[j].pid == sr_b[i].pid) {
				dt[i] = sr_b[i].t > sr_a[j].t ? sr_b[i].t - sr_a[j].t : 0;
				break;
			}
		top += dt[i];
	}
	for (int k = 0; k < 9; k++) {	/* en buyuk 9 (secmeli siralama) */
		int en = -1;
		for (int i = 0; i < bn; i++)
			if (dt[i] != (unsigned long long)-1 && (en < 0 || dt[i] > dt[en]))
				en = i;
		if (en < 0 || dt[en] == (unsigned long long)-1)
			break;
		char m[96];
		snprintf(m, sizeof m, "%5d %c %-16s %5.1f%% %6ld KB", sr_b[en].pid, sr_b[en].durum, sr_b[en].ad,
			 100.0 * (double)dt[en] / (double)dc, sr_b[en].rss);	/* 0.60: durum harfi */
		gunluk_satir("surec", m);
		dt[en] = (unsigned long long)-1;
	}
	char m[96];
	snprintf(m, sizeof m, "ozet: surecler %.0f%% CPU (1 sn), %d surec", 100.0 * (double)top / (double)dc, bn);
	gunluk_satir("surec", m);
	{	/* 0.60: yuk kaynagi (G/C bekleyen surecler) */
		char y[160], la[96] = "?";
		proc_yol(y, sizeof y, "/proc/loadavg");
		if (dosya_oku(y, la, sizeof la) > 0) {
			char *sp = la;
			for (int k = 0; k < 3 && sp; k++)
				sp = strchr(sp + 1, ' ');
			if (sp)
				*sp = 0;
		}
		char yk[150];
		snprintf(yk, sizeof yk, "yuk: %.40s · calisan %ld · G/C bekleyen %ld · iowait %.0f%% (1 sn)", la,
			 sr_stat_sayi("procs_running "), sr_stat_sayi("procs_blocked "), 100.0 * (double)diow / (double)dc);
		gunluk_satir("surec", yk);
		char dl[150] = "D:";
		int nd = 0;
		for (int i = 0; i < bn; i++)
			if (sr_b[i].durum == 'D') {
				size_t u = strlen(dl);
				if (u < 120)
					snprintf(dl + u, sizeof dl - u, " %.20s(%d)", sr_b[i].ad, sr_b[i].pid);
				nd++;
			}
		if (!nd)
			snprintf(dl, sizeof dl, "D: yok (o an G/C bekleyen surec yok)");
		gunluk_satir("surec", dl);
		char mi[160] = "", bl[96];
		proc_yol(y, sizeof y, "/proc/meminfo");
		const char *p = dosya_oku(y, mi, sizeof mi) > 0 ? strstr(mi, "MemAvailable:") : NULL;
		snprintf(bl, sizeof bl, "bellek: kullanilabilir %ld KB", p ? atol(p + 13) : -1L);
		gunluk_satir("surec", bl);
	}
	printf("surec tablosu gonderildi (%d surec)\n", bn);
}

static void sr_baslat(void)
{
	if (sr_tmr)
		return;
	sr_cpu_a = sr_cpu_toplam();
	sr_iow_a = sr_iow_son;	/* 0.60 */
	sr_an = sr_oku(sr_a, SR_AZAMI);
	sr_tmr = lv_timer_create(sr_bitir_cb, 1000, NULL);
	lv_timer_set_repeat_count(sr_tmr, 1);
}

static void bk_yaz(const char *metin);	/* 0.57: Telegram kuyrugu (asagida) */

static void *net_thread(void *arg)
{
	int ls, one = 1, deneme = 0;
	struct sockaddr_in a = {.sin_family = AF_INET, .sin_port = htons(PORT)};

	(void)arg;
	/* 0.79 (denetim v3 A1): CLOEXEC yoktu; system("wifi-kaydet") ile baslayan wpa_supplicant 8765'i miras alip tutuyor,
	 * panel yeniden baslayinca bind basarisiz olup is parcacigi vazgeciyordu. Artik 2 sn'de bir yeniden denenir. */
	for (;;) {
		ls = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
		setsockopt(ls, SOL_SOCKET, SO_REUSEADDR, &one, sizeof one);
		if (ls >= 0 && !bind(ls, (struct sockaddr *)&a, sizeof a) && !listen(ls, 2))
			break;
		if (!deneme++)
			perror("dinleme (2 sn'de bir yeniden denenecek)");
		if (ls >= 0)
			close(ls);
		sleep(2);
	}
	if (deneme)
		printf("dinleme: port %d %d. denemede alindi\n", PORT, deneme + 1);
	for (;;) {
		struct sockaddr_in ca;
		socklen_t cl = sizeof ca;
		int c = accept4(ls, (struct sockaddr *)&ca, &cl, SOCK_CLOEXEC);	/* 0.79: baglanti da sureclere gecmesin */
		if (c < 0)
			continue;
		if (!izinli_soket(c, NULL)) {	/* 0.77: /data/izinli-ip disindaki LAN cihazi */
			static in_addr_t son_red;	/* inceleme: ayni adres 2-5 sn'de bir denerse panel.log buyumesin */
			if (ca.sin_addr.s_addr != son_red)
				printf("PC baglantisi reddedildi: %s (/data/izinli-ip)\n", inet_ntoa(ca.sin_addr));
			son_red = ca.sin_addr.s_addr;
			close(c);
			continue;
		}
		setsockopt(c, IPPROTO_TCP, TCP_NODELAY, &one, sizeof one);
		snprintf(pc_peer, sizeof pc_peer, "%s", inet_ntoa(ca.sin_addr));
		pc_conn = 1;
		pc_last = mono_s();
		printf("PC baglandi: %s\n", pc_peer);
		pthread_mutex_lock(&mu); outn = 0; pthread_mutex_unlock(&mu);
		char hello[96];
		snprintf(hello, sizeof hello, "{\"t\":\"hello\",\"ver\":\"%s\"}\n", PANEL_VER);
		send(c, hello, strlen(hello), MSG_NOSIGNAL);
		int atla = 0;	/* 0.80: asiri uzun satirin geri kalani atiliyor */
		size_t cap = 1 << 14, len = 0;	/* 0.79: 16 KB'tan baslar, satir en cok 64 KB (denetim v3 A12) */
		char *buf = malloc(cap);
		for (;;) {
			struct pollfd pf = {.fd = c, .events = POLLIN};
			int r = poll(&pf, 1, 100);
			if (r > 0) {
				if (len + 4096 > cap) {
					/* 0.79: 1 MB satir cJSON ile ~40 MB yigin tutuyordu (gercek satirlar 1-5 KB); realloc denetlenir */
					char *nb = cap >= (1 << 16) ? NULL : realloc(buf, cap * 2);
					if (nb) {
						buf = nb;
						cap *= 2;
					} else {
						len = 0;	/* asiri uzun satir ya da bellek yok: at (eski tampon kalir) */
						atla = 1;	/* 0.80: geri kalani yeni satir sanilmasin */
					}
				}
				ssize_t n = recv(c, buf + len, cap - len - 1, 0);
				if (n <= 0)
					break;
				len += n;
				buf[len] = 0;
				if (atla) {	/* 0.80 (denetim v3 A12 kalani): '\n'e kadar at */
					char *son = memchr(buf, '\n', len);
					if (son) {
						atla = 0;
						len -= (size_t)(son + 1 - buf);
						memmove(buf, son + 1, len + 1);
					} else {
						len = 0;
					}
				}
				char *nl;
				while ((nl = memchr(buf, '\n', len))) {
					*nl = 0;
					if (nl > buf) {
						pthread_mutex_lock(&mu);
						if (strstr(buf, "\"t\":\"ui\"") || strstr(buf, "\"t\": \"ui\"")) {
							if (ui_n < 8)
								ui_pending[ui_n++] = strdup(buf);
						} else {
							free(pending);
							pending = strdup(buf);
						}
						pthread_mutex_unlock(&mu);
						pc_last = mono_s();
					}
					len -= (nl + 1 - buf);
					memmove(buf, nl + 1, len);
				}
			}
			pthread_mutex_lock(&mu);
			int n = outn;
			char q[16][192];
			memcpy(q, outq, sizeof q);
			outn = 0;
			pthread_mutex_unlock(&mu);
			for (int i = 0; i < n; i++)
				if (send(c, q[i], strlen(q[i]), MSG_NOSIGNAL) < 0)
					break;
			if (mono_s() - pc_last > 30)
				break;	/* 30 sn veri yok: baglantiyi birak */
		}
		free(buf);
		close(c);
		pc_conn = 0;
		printf("PC baglantisi kapandi\n");
	}
	return NULL;
}

/* ----------------------------------------------------------------- yazitipi */
static lv_font_t *f_s, *f_m, *f_mb, *f_l, *f_xl, *f_clock;

LV_FONT_DECLARE(ubuntu_r22)
LV_FONT_DECLARE(ubuntu_r28)
LV_FONT_DECLARE(ubuntu_b28)
LV_FONT_DECLARE(ubuntu_b40)
LV_FONT_DECLARE(ubuntu_b64)
LV_FONT_DECLARE(ubuntu_b150)
static lv_font_t F_s, F_m, F_mb, F_l, F_xl, F_clock;

static lv_font_t *fcopy(lv_font_t *dst, const lv_font_t *src, const lv_font_t *fb)
{
	*dst = *src;
	dst->fallback = fb;
	return dst;
}

static void fonts_init(void)
{
	f_s = fcopy(&F_s, &ubuntu_r22, &lv_font_montserrat_20);
	f_m = fcopy(&F_m, &ubuntu_r28, &lv_font_montserrat_28);
	f_mb = fcopy(&F_mb, &ubuntu_b28, &lv_font_montserrat_28);
	f_l = fcopy(&F_l, &ubuntu_b40, &lv_font_montserrat_28);
	f_xl = fcopy(&F_xl, &ubuntu_b64, &lv_font_montserrat_28);
	f_clock = fcopy(&F_clock, &ubuntu_b150, &lv_font_montserrat_28);
	printf("yazitipi: Ubuntu bitmap (22/28/40/64/150)\n");
}

/* ---------------------------------------------------------------------- arayuz */
static const char *gunler[] = {"Pazar", "Pazartesi", "Salı", "Çarşamba", "Perşembe", "Cuma", "Cumartesi"};
static const char *aylar[] = {"Ocak", "Şubat", "Mart", "Nisan", "Mayıs", "Haziran", "Temmuz",
			      "Ağustos", "Eylül", "Ekim", "Kasım", "Aralık"};

/* Temalar: renkler tek tablodan; C_* makrolari etkin temayi okur */
typedef struct {
	const char *ad;
	uint32_t bg, card, acc, acc2, text, dim, bar, btn, btn2, sep, toast, toast_text;
	int dark;
} tema_t;
static const tema_t temalar[] = {
	{"Mavi", 0x0B1220, 0x152036, 0x3B82F6, 0x0EA5E9, 0xE5E7EB, 0x94A3B8, 0x0F1A2E, 0x1F2A44, 0x1E3A5F, 0x24324D,
	 0x1F2937, 0xF9FAFB, 1},
	{"Kırmızı & Siyah", 0x050505, 0x161010, 0xE11D48, 0xF97316, 0xF5F5F5, 0xA8A29E, 0x0E0909, 0x2A1414, 0x3A1418,
	 0x3A1E1E, 0x262626, 0xFAFAFA, 1},
	{"Beyaz", 0xEEF2F7, 0xFFFFFF, 0x2563EB, 0x0891B2, 0x0F172A, 0x64748B, 0xE2E8F0, 0xE2E8F0, 0xDBEAFE, 0xCBD5E1,
	 0x1E293B, 0xF8FAFC, 0},
};
#define TEMA_N ((int)(sizeof temalar / sizeof temalar[0]))
static int tema;
#define TH (temalar[tema])
#define C_BG lv_color_hex(TH.bg)
#define C_CARD lv_color_hex(TH.card)
#define C_ACC lv_color_hex(TH.acc)
/* 0.22: acik (Beyaz) temada durum renkleri koyulasir (beyaz zeminde okunurluk) */
#define C_OK lv_color_hex(TH.dark ? 0x22C55E : 0x15803D)
#define C_WARN lv_color_hex(TH.dark ? 0xF59E0B : 0xB45309)
#define C_BAD lv_color_hex(TH.dark ? 0xEF4444 : 0xDC2626)
#define C_DIM lv_color_hex(TH.dim)

static lv_obj_t *l_hdr_date, *l_hdr_net;
static lv_obj_t *l_clock, *l_sec, *l_date, *l_weather, *l_weather2;
static lv_obj_t *l_pcname, *b_cpu, *l_cpu, *b_ram, *l_ram, *b_disk, *l_disk, *l_netio, *l_pcup;
static lv_obj_t *l_fx, *l_events;
/* 0.32 yeni Pano: hava karti + durum karti */
static lv_obj_t *hv_ikon, *hv_sehir, *hv_sicak, *hv_durum, *hv_detay, *hv_yarin, *hv_saat, *pd_l;
static int hv_kod = -1;
/* 0.35 namaz vakitleri ve haber basliklari (panel-veri.json) */
static lv_obj_t *pn_namaz, *pn_haber;
static char pn_vakit[6][6], pn_haber_l[8][160];
static int pn_haber_n, pn_haber_i;
static const char *const pn_vakit_ad[6] = {"İmsak", "Güneş", "Öğle", "İkindi", "Akşam", "Yatsı"};
static time_t pv_zaman, pv_mtime;
static lv_obj_t *macro_cont, *prompt_cont, *l_answer, *l_asst_state, *l_asst_soru;	/* 0.69 l_asst_soru */
#define AS_SORU_MAX 200	/* 0.69: serbest soru en cok (bayt) */
static lv_obj_t *l_sys, *l_bt, *toast, *tabview;
static lv_obj_t *pc_image, *l_pc_image, *pc_full, *pc_full_img, *pc_bar, *l_pc_bar;
static pc_session_t pc_session;
static uint32_t pc_request_seq;
static char pc_status[160] = "PC ajanı bekleniyor";
static pc_shm_t pc_shm;
static lv_image_dsc_t pc_dsc;
static uint32_t pc_updates, pc_inv_rects, pc_inv_full, pc_dismiss_gen, pc_dismiss_conn, pc_bar_until, pc_last_alive;
static int pc_dismissed;
static char macro_sig[2048], prompt_sig[2048];
static char btn_ids[2][MAX_BTN][48];
static char btn_lbl[2][MAX_BTN][96];	/* 0.52: etiket = cihazda soru metni */
static double pc_ajan_t;	/* 0.52: gercek PC ajanindan son "prompts" */
static double pc_state_t;	/* 0.58: PC ajanindan son "state" (pc metrikleri; ajan v7+ prompts gondermeyebilir) */
static char pc_ses[8];	/* 0.58: PC ajaninin bildirdigi ikinci ekran ses yonu ("panel" | "pc") */

static lv_obj_t *label(lv_obj_t *par, const lv_font_t *f, const char *txt)
{
	lv_obj_t *l = lv_label_create(par);

	lv_obj_set_style_text_font(l, f, 0);
	lv_label_set_text(l, txt);
	return l;
}

/* 0.22: duz (stilsiz) yatay/dikey kutu */
static lv_obj_t *kutu(lv_obj_t *par, lv_flex_flow_t akis, int bosluk)
{
	lv_obj_t *b = lv_obj_create(par);

	lv_obj_remove_style_all(b);
	lv_obj_set_size(b, LV_PCT(100), LV_SIZE_CONTENT);
	lv_obj_set_flex_flow(b, akis);
	lv_obj_set_style_pad_gap(b, bosluk, 0);
	lv_obj_remove_flag(b, LV_OBJ_FLAG_SCROLLABLE);
	return b;
}

/* 0.22: kart basligi = vurgu renkli simge + aralikli kucuk baslik; kart 1 px kenarlikli */
static lv_obj_t *card_i(lv_obj_t *par, const char *icon, const char *title)
{
	lv_obj_t *c = lv_obj_create(par);

	lv_obj_set_style_bg_color(c, lv_color_mix(lv_color_hex(TH.sep), C_CARD, 40), 0);	/* 0.45: ustten hafif gradyan */
	lv_obj_set_style_bg_grad_color(c, C_CARD, 0);
	lv_obj_set_style_bg_grad_dir(c, LV_GRAD_DIR_VER, 0);
	lv_obj_set_style_bg_main_stop(c, 0, 0);
	lv_obj_set_style_bg_grad_stop(c, 140, 0);
	lv_obj_set_style_border_width(c, 1, 0);
	lv_obj_set_style_border_color(c, lv_color_hex(TH.sep), 0);
	lv_obj_set_style_radius(c, 20, 0);
	lv_obj_set_style_pad_all(c, 20, 0);
	lv_obj_set_flex_flow(c, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_style_pad_row(c, 10, 0);
	lv_obj_set_height(c, LV_PCT(100));
	lv_obj_set_flex_grow(c, 1);
	lv_obj_remove_flag(c, LV_OBJ_FLAG_SCROLLABLE);
	if (title) {
		lv_obj_t *r = kutu(c, LV_FLEX_FLOW_ROW, 10);
		lv_obj_set_flex_align(r, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
		lv_obj_set_style_pad_bottom(r, 2, 0);
		if (icon) {
			lv_obj_t *ic = label(r, f_s, icon);
			lv_obj_set_style_text_color(ic, C_ACC, 0);
		}
		lv_obj_t *t = label(r, f_s, title);
		lv_obj_set_style_text_color(t, C_DIM, 0);
		lv_obj_set_style_text_letter_space(t, 2, 0);
	}
	return c;
}

static lv_obj_t *card(lv_obj_t *par, const char *title)
{
	return card_i(par, NULL, title);
}

/* 0.22: hap gosterge (ust serit / koruyucu): renkli nokta + yazi. dolu=1 ise zemin renkli, yazi beyaz. */
typedef struct { lv_obj_t *kap, *nokta, *yazi; uint32_t son_renk; int son_dolu; } hap_t;

static hap_t hap(lv_obj_t *par)
{
	hap_t h = {0};
	h.son_dolu = -1;
	h.kap = lv_obj_create(par);
	lv_obj_remove_style_all(h.kap);
	lv_obj_set_size(h.kap, LV_SIZE_CONTENT, 38);
	lv_obj_set_style_radius(h.kap, 19, 0);
	lv_obj_set_style_pad_hor(h.kap, 14, 0);
	lv_obj_set_style_bg_opa(h.kap, LV_OPA_COVER, 0);
	lv_obj_set_style_bg_color(h.kap, lv_color_hex(TH.btn), 0);
	lv_obj_set_flex_flow(h.kap, LV_FLEX_FLOW_ROW);
	lv_obj_set_flex_align(h.kap, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	lv_obj_set_style_pad_column(h.kap, 8, 0);
	lv_obj_remove_flag(h.kap, LV_OBJ_FLAG_SCROLLABLE);
	h.nokta = lv_obj_create(h.kap);
	lv_obj_remove_style_all(h.nokta);
	lv_obj_set_size(h.nokta, 10, 10);
	lv_obj_set_style_radius(h.nokta, 5, 0);
	lv_obj_set_style_bg_opa(h.nokta, LV_OPA_COVER, 0);
	lv_obj_set_style_bg_color(h.nokta, C_DIM, 0);
	h.yazi = label(h.kap, f_s, "");
	return h;
}

static hap_t h_pc, h_priv, h_kalkan;	/* ust serit haplari (0.22) */
static lv_obj_t *wf_bar[4];
static int wf_bar_duzey = -1;

static void hap_ayarla(hap_t *h, lv_color_t renk, int dolu, const char *yazi)
{
	if (!h->kap)
		return;
	uint32_t rk = lv_color_to_u32(renk);
	if (h->son_dolu == dolu && h->son_renk == rk && !strcmp(lv_label_get_text(h->yazi), yazi))
		return;	/* degisiklik yok: yeniden cizim tetikleme */
	h->son_dolu = dolu;
	h->son_renk = rk;
	lv_label_set_text(h->yazi, yazi);
	lv_obj_set_style_bg_color(h->kap, dolu ? renk : lv_color_hex(TH.btn), 0);
	lv_obj_set_style_bg_color(h->nokta, dolu ? lv_color_white() : renk, 0);
	lv_obj_set_style_text_color(h->yazi, dolu ? lv_color_white() : lv_color_hex(TH.text), 0);
}

static lv_obj_t *metric(lv_obj_t *par, const char *name, lv_obj_t **bar)
{
	lv_obj_t *row = lv_obj_create(par), *l;

	lv_obj_remove_style_all(row);
	lv_obj_set_size(row, LV_PCT(100), LV_SIZE_CONTENT);
	lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
	lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	lv_obj_t *n = label(row, f_m, name);
	lv_obj_set_style_text_color(n, C_DIM, 0);
	l = label(row, f_mb, "--");
	*bar = lv_bar_create(par);
	lv_obj_set_size(*bar, LV_PCT(100), 12);
	lv_bar_set_range(*bar, 0, 100);
	lv_obj_set_style_bg_color(*bar, lv_color_hex(TH.sep), LV_PART_MAIN);
	lv_obj_set_style_bg_opa(*bar, LV_OPA_COVER, LV_PART_MAIN);
	lv_obj_set_style_radius(*bar, 6, LV_PART_MAIN);
	lv_obj_set_style_radius(*bar, 6, LV_PART_INDICATOR);
	lv_obj_set_style_anim_duration(*bar, 400, LV_PART_MAIN);
	return l;
}

/* 0.22: bildirim 180 ms'de belirir/kaybolur. Kaybolma bitince GIZLI yapilir (dogrudan PC cizimi gizli olmayan
 * ust katman nesnesi varken LVGL yoluna duser; saydam ama gizli olmayan nesne birakilmamali). */
static void toast_opa_cb(void *o, int32_t v)
{
	lv_obj_set_style_opa((lv_obj_t *)o, (lv_opa_t)v, 0);
}

static void toast_gizle_cb(lv_anim_t *a)
{
	lv_obj_t *o = (lv_obj_t *)a->var;
	if (!lv_obj_get_user_data(o))	/* bu arada yeni bildirim gelmediyse */
		lv_obj_add_flag(o, LV_OBJ_FLAG_HIDDEN);
	lv_obj_set_style_opa(o, LV_OPA_COVER, 0);
}

static void toast_anim(int ac)
{
	lv_anim_t a;
	lv_anim_delete(toast, toast_opa_cb);
	lv_anim_init(&a);
	lv_anim_set_var(&a, toast);
	lv_anim_set_exec_cb(&a, toast_opa_cb);
	lv_anim_set_duration(&a, 180);
	lv_anim_set_values(&a, ac ? LV_OPA_TRANSP : LV_OPA_COVER, ac ? LV_OPA_COVER : LV_OPA_TRANSP);
	lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
	if (!ac)
		lv_anim_set_completed_cb(&a, toast_gizle_cb);
	lv_anim_start(&a);
}

/* 0.23 bildirim gecmisi (RAM, son BLD_N); Denetim Merkezi ve Ayarlar > Sistem gosterir */
#define BLD_N 8
static struct { time_t t; int sessiz; char m[96]; } bld[BLD_N];
static int bld_bas, bld_say;
static uint32_t bld_surum;	/* degisince gostergeler yenilenir */
static int dnd;	/* Rahatsiz etme (panel.conf rahatsiz_etme) */
static int gece_ayar = 1, gece_aktif;	/* 0.41 gece kipi: ayar (/data/panel-gece-kapali yoksa 1) ve su an etkin mi */

static void bildirim_kaydet(const char *txt, int sessiz)
{
	int i = (bld_bas + bld_say) % BLD_N;
	if (bld_say == BLD_N)
		bld_bas = (bld_bas + 1) % BLD_N;
	else
		bld_say++;
	bld[i].t = time(NULL);
	bld[i].sessiz = sessiz;
	not_temizle(bld[i].m, sizeof bld[i].m, txt);	/* tek satir, UTF-8 sinirinda kesik */
	bld_surum++;
}

/* k = 0 en yeni */
static int bildirim_al(int k, char *o, size_t n)
{
	if (k < 0 || k >= bld_say)
		return -1;
	int i = (bld_bas + bld_say - 1 - k) % BLD_N;
	struct tm tm;
	localtime_r(&bld[i].t, &tm);
	snprintf(o, n, "%02d:%02d  %s%s", tm.tm_hour, tm.tm_min, bld[i].m, bld[i].sessiz ? "  (sessiz)" : "");
	return 0;
}

static void show_toast(const char *txt)
{
	bildirim_kaydet(txt, 0);
	int zaten = !lv_obj_has_flag(toast, LV_OBJ_FLAG_HIDDEN) && lv_obj_get_user_data(toast);
	lv_label_set_text(toast, txt);
	lv_obj_remove_flag(toast, LV_OBJ_FLAG_HIDDEN);
	lv_obj_move_foreground(toast);
	lv_obj_set_user_data(toast, (void *)(uintptr_t)(tick_ms() + 2500));
	if (!zaten)
		toast_anim(1);
	else {
		lv_anim_delete(toast, toast_opa_cb);
		lv_obj_set_style_opa(toast, LV_OPA_COVER, 0);
	}
}

/* 0.23: PC'den gelen bildirim; Rahatsiz etme acikken gosterilmez, yalniz gecmise yazilir */
static void rg_bildirim_olay(void);	/* 0.24: robot goz acikken saskin bakis (asagida) */
static void pc_toast(const char *txt)
{
	if (dnd || gece_aktif)	/* 0.41: gece kipi de bildirimi gizler (kaydetmeden) */
		bildirim_kaydet(txt, 1);
	else {
		show_toast(txt);
		rg_bildirim_olay();
	}
}

/* 0.23: PC makro olayi (Kisayollar ile ayni {"t":"macro","id":...}; PC ajani isler) */
static void makro_gonder(const char *id)
{
	char j[128];
	snprintf(j, sizeof j, "{\"t\":\"macro\",\"id\":\"%s\"}", id);
	send_event(j);
	show_toast(pc_conn ? LV_SYMBOL_OK "  Gönderildi" : LV_SYMBOL_WARNING "  PC bağlı değil, gönderilmedi");
}

/* ---------------------------------------------------------------- 0.52 PC'siz asistan (cihazdaki kole-gemini) */
static pid_t as_pid = -1;
static int as_fd = -1;
static char as_cikti[8192];
static size_t as_n;
static uint32_t as_t0;
static lv_timer_t *as_tmr;

/* gercek PC ajani: baglanti + son 90 sn'de istem listesi (tani araclari da 8765'e baglanir, onlar sayilmaz) */
static int pc_ajan_var(void)
{
	return pc_conn && pc_ajan_t > 0 && mono_s() - pc_ajan_t < 90;
}

/* 0.58: PC ajani bagli mi (istem listesinden bagimsiz; ajan v7/v8 her 2 sn "state" gonderir) */
static int pc_ajan_bagli(void)
{
	return pc_conn && pc_state_t > 0 && mono_s() - pc_state_t < 15;
}

static const char *as_gemini_yol(char *b, size_t n)
{
	const char *e = getenv("PANEL_GEMINI");
	if (e && *e)
		return e;
	DIR *d = opendir("/mnt/sd/velinux/asistan");
	int en = -1;
	if (!d)
		return NULL;
	struct dirent *g;
	while ((g = readdir(d))) {
		int v;
		char son;
		if (sscanf(g->d_name, "kole-gemini-v%d%c", &v, &son) == 1 && v > en) {
			char y[300];
			snprintf(y, sizeof y, "/mnt/sd/velinux/asistan/%s", g->d_name);
			if (!access(y, X_OK)) {
				en = v;
				snprintf(b, n, "%s", y);
			}
		}
	}
	closedir(d);
	return en >= 0 ? b : NULL;
}

/* 0.69: cihazda kole-gemini + Gemini anahtari var mi (Asistan once bunu kullanir); 30 sn onbellek (SD dizini okunur) */
static int as_gemini_var(void)
{
	static int var = -1;
	static uint32_t t;
	if (var < 0 || tick_ms() - t > 30000) {
		char b[320];
		var = as_gemini_yol(b, sizeof b) != NULL && access("/data/asistan/gemini-anahtar", R_OK) == 0;
		t = tick_ms();
	}
	return var;
}

static void as_temizle(void)
{
	if (as_fd >= 0)
		close(as_fd);
	as_fd = -1;
	as_pid = -1;
	if (as_tmr)
		lv_timer_delete(as_tmr);
	as_tmr = NULL;
}

static void as_tmr_cb(lv_timer_t *t)
{
	(void)t;
	ssize_t r;
	while (as_fd >= 0 && as_n < sizeof as_cikti - 1 && (r = read(as_fd, as_cikti + as_n, sizeof as_cikti - 1 - as_n)) > 0)
		as_n += (size_t)r;
	as_cikti[as_n] = 0;
	int st = 0;
	pid_t w = waitpid(as_pid, &st, WNOHANG);
	uint32_t ms = tick_ms() - as_t0;
	if (w == 0) {
		if (ms > 45000) {
			kill(as_pid, SIGKILL);
			waitpid(as_pid, &st, 0);
			printf("asistan (cihaz): 45 sn zaman asimi\n");
			lv_label_set_text(l_asst_state, "Cihazdaki asistan zamanında yanıt vermedi.");
			as_temizle();
		}
		return;
	}
	int rc = WIFEXITED(st) ? WEXITSTATUS(st) : 99;
	cJSON *j = rc == 0 ? cJSON_Parse(as_cikti) : NULL;
	cJSON *c = j ? cJSON_GetObjectItem(j, "cevap") : NULL;
	if (cJSON_IsString(c) && c->valuestring[0]) {
		lv_label_set_text(l_answer, c->valuestring);
		lv_label_set_text_fmt(l_asst_state, "Gemini yanıtladı (%.1f sn) · cihazda, Wi-Fi", ms / 1000.0);	/* 0.69 */
	} else {
		lv_label_set_text(l_asst_state, rc == 3 ? "Cihazda Gemini anahtarı yok." : "Cihazdaki asistan yanıt veremedi (bağlantı?).");
	}
	printf("asistan (cihaz): rc %d, %zu bayt, %u ms\n", rc, as_n, ms);
	cJSON_Delete(j);
	memset(as_cikti, 0, sizeof as_cikti);
	as_temizle();
}

/* istem metnini cihazdaki kole-gemini'ye sor (arka planda); 0 basladi */
static int as_cihazda_sor(const char *soru)
{
	char yb[320];
	const char *yol = as_gemini_yol(yb, sizeof yb);
	if (as_pid > 0) {
		show_toast(LV_SYMBOL_WARNING "  Önceki soru sürüyor");
		return -1;
	}
	if (!yol || !soru || !*soru) {
		lv_label_set_text(l_asst_state, "PC bağlı değil ve cihazda asistan (kole-gemini) bulunamadı.");
		return -1;
	}
	int p[2];
	if (pipe2(p, O_CLOEXEC))	/* 0.79: baska is parcaciginin system()'i yazma ucunu alirsa EOF gelmezdi */
		return -1;
	pid_t c = fork();
	if (c == 0) {
		dup2(p[1], 1);
		int bos = open("/dev/null", O_RDWR);
		if (bos >= 0)
			dup2(bos, 2);
		for (int f = 3; f < 1024; f++)
			close(f);
		setenv("SSL_CERT_FILE", "/mnt/sd/velinux/tailscale/ca-certificates.crt", 0);
		setenv("GOMEMLIMIT", "12MiB", 1);
		setenv("GOGC", "50", 1);
		execl(yol, "kole-gemini", "--metin", soru, (char *)NULL);
		_exit(127);
	}
	close(p[1]);
	if (c < 0) {
		close(p[0]);
		return -1;
	}
	fcntl(p[0], F_SETFL, O_NONBLOCK);
	as_pid = c;
	as_fd = p[0];
	as_n = 0;
	as_t0 = tick_ms();
	as_tmr = lv_timer_create(as_tmr_cb, 200, NULL);
	lv_label_set_text(l_asst_state, "Gemini düşünüyor…");	/* 0.69 */
	if (l_asst_soru)
		lv_label_set_text_fmt(l_asst_soru, "Soru: %s", soru);
	lv_label_set_text(l_answer, "");
	printf("asistan (cihaz): soru %zu bayt\n", strlen(soru));
	return 0;
}

static void btn_cb(lv_event_t *e)
{
	uintptr_t v = (uintptr_t)lv_event_get_user_data(e);
	int kind = v >> 8, idx = v & 0xFF;
	char j[160];
	cJSON *o = cJSON_CreateObject();

	if (kind && (as_gemini_var() || !pc_ajan_var())) {	/* 0.69: once cihazdaki Gemini (0.52: yalniz PC yokken) */
		as_cihazda_sor(btn_lbl[1][idx]);
		cJSON_Delete(o);
		return;
	}
	cJSON_AddStringToObject(o, "t", kind ? "ask" : "macro");
	cJSON_AddStringToObject(o, "id", btn_ids[kind][idx]);
	char *s = cJSON_PrintUnformatted(o);
	snprintf(j, sizeof j, "%s", s);
	free(s);
	cJSON_Delete(o);
	send_event(j);
	if (!pc_conn)
		show_toast(LV_SYMBOL_WARNING "  PC bağlı değil, gönderilmedi");
	else if (kind)
		lv_label_set_text(l_asst_state, "Yanıt bekleniyor...");
	else
		show_toast(LV_SYMBOL_OK "  Gönderildi");
}

/* id/label listesinden buton izgarasi kur (kind 0 makro, 1 asistan istemi) */
static void build_buttons(lv_obj_t *cont, cJSON *arr, int kind, int bw, int bh)
{
	cJSON *it;
	int i = 0;

	lv_obj_clean(cont);
	cJSON_ArrayForEach(it, arr) {
		cJSON *id = cJSON_GetObjectItem(it, "id"), *lb = cJSON_GetObjectItem(it, "label");
		if (!cJSON_IsString(id) || !cJSON_IsString(lb) || i >= MAX_BTN)
			continue;
		snprintf(btn_ids[kind][i], sizeof btn_ids[0][0], "%s", id->valuestring);
		snprintf(btn_lbl[kind][i], sizeof btn_lbl[0][0], "%s", lb->valuestring);
		lv_obj_t *b = lv_button_create(cont);
		lv_obj_set_size(b, bw, bh);
		lv_obj_set_style_radius(b, 16, 0);
		lv_obj_set_style_bg_color(b, kind ? lv_color_hex(TH.btn2) : lv_color_hex(TH.btn), 0);
		lv_obj_set_style_bg_color(b, C_ACC, LV_STATE_PRESSED);
		lv_obj_set_style_border_width(b, 1, 0);	/* 0.22: kartlarla ayni ince kenar */
		lv_obj_set_style_border_color(b, lv_color_hex(TH.sep), 0);
		lv_obj_set_style_shadow_width(b, 0, 0);	/* yazilim golgesi pahali */
		lv_obj_set_style_text_color(b, lv_color_hex(TH.text), 0);	/* Beyaz temada okunurluk */
		lv_obj_set_style_text_color(b, lv_color_white(), LV_STATE_PRESSED);
		lv_obj_t *l = label(b, kind ? f_m : f_mb, lb->valuestring);
		lv_label_set_long_mode(l, LV_LABEL_LONG_WRAP);
		lv_obj_set_width(l, bw - 24);
		lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_CENTER, 0);
		lv_obj_center(l);
		lv_obj_add_event_cb(b, btn_cb, LV_EVENT_CLICKED, (void *)(uintptr_t)((kind << 8) | i));
		i++;
	}
}

static const char *default_macros =
	"[{\"id\":\"playpause\",\"label\":\"" LV_SYMBOL_PLAY " Oynat / Duraklat\"},"
	"{\"id\":\"prev\",\"label\":\"" LV_SYMBOL_PREV " Önceki\"},"
	"{\"id\":\"next\",\"label\":\"" LV_SYMBOL_NEXT " Sonraki\"},"
	"{\"id\":\"mute\",\"label\":\"" LV_SYMBOL_MUTE " Sessiz\"},"
	"{\"id\":\"voldown\",\"label\":\"" LV_SYMBOL_VOLUME_MID " Ses -\"},"
	"{\"id\":\"volup\",\"label\":\"" LV_SYMBOL_VOLUME_MAX " Ses +\"},"
	"{\"id\":\"lock\",\"label\":\"" LV_SYMBOL_EYE_CLOSE " Ekranı kilitle\"},"
	"{\"id\":\"calc\",\"label\":\"" LV_SYMBOL_EDIT " Hesap makinesi\"}]";
static const char *default_prompts =
	"[{\"id\":\"ozet\",\"label\":\"Günün kısa özeti\"},{\"id\":\"hava\",\"label\":\"Bugün hava nasıl olacak?\"},"
	"{\"id\":\"doviz\",\"label\":\"Dolar ve euro kaç lira?\"},{\"id\":\"haber\",\"label\":\"Kısa haber özeti\"},"
	"{\"id\":\"siber\",\"label\":\"Günün siber haberleri\"},"	/* 0.73 */
	"{\"id\":\"saka\",\"label\":\"Bana kısa bir şaka yap\"},{\"id\":\"soz\",\"label\":\"Güne motive edici bir söz\"}]";
	/* 0.52: etiketler cihazda (PC ajani yokken) dogrudan soru metni; PC ajani gelince kendi listesi yerini alir */

/* Parlaklik: otomatik (PC ajaninin gece/gunduz degeri, PC yoksa panelin kendi saati) ya da manuel (%5..%100).
 * Secim /data/panel.conf'ta kalici (flash yipranmasin diye yalniz anahtar degisince ve cubuk birakilinca yazilir). */
static int br_auto = 1, br_manual = 80, br_pc = -1, br_cur = -1, ss_on = 1, ss_shown;
static int ss_dk = 3, ses_duzey = 60, ses_sessiz;	/* 0.23 (panel_ayar_v1.h); dnd yukarida */
static int ss_tur = 1;	/* koruyucu turu (panel_ayar_v3.h): 0 saat, 1 robot goz, 2 karisik, 0.40: 3 analog, 4 logo, 5 bilgi, 6 karartma */
static lv_obj_t *sw_auto, *br_box, *sl_br, *l_br_pct, *l_br_auto;
static lv_obj_t *ss_obj, *tm_label, *wf_win;	/* koruyucu/zamanlayici (asagida tanimli), yeniden kurulumda sifirlanir */
static lv_obj_t *ss_box, *gz_mb;	/* 0.22 (asagida tanimli) */
static hap_t ss_priv;

static int pct_to_raw(int p) { return 3 + (p * 252 + 50) / 100; }
static int raw_to_pct(int r) { return (r - 3) * 100 / 252; }

/* 0.23: ayarlar panel_ayar_v1.h ile (0.22 dosyasiyla iki yonlu uyumlu); yalniz main'de bir kez okunur */
static void br_load(void)
{
	struct panel_ayar a;
	char yol[160], buf[1024];

	panel_ayar_varsayilan(&a);
	veri_yol(yol, sizeof yol, "panel.conf");
	if (dosya_oku(yol, buf, sizeof buf) > 0)
		panel_ayar_coz(&a, buf, TEMA_N);
	br_auto = a.br_auto;
	br_manual = a.br_manual;
	tema = a.tema;
	ss_on = a.ss_on;
	ss_dk = a.ss_dk;
	ses_duzey = a.ses;
	ses_sessiz = a.sessiz;
	dnd = a.dnd;
	ss_tur = a.ss_tur;
}

/* 0.39: panel ses ayari -> F133 codec (ses_kart_v1.h); MCU'ya gonderim yok. Yalniz degisince yazar/gunluge yazar. */
static void ses_codec_uygula(void)
{
	static int son_r = 1, son_hedef = -1, son_sessiz = -1;
	int hedef = sk_yuzde_seviye(ses_duzey);
	int r = sk_uygula(ses_duzey, ses_sessiz);
	if (hedef != son_hedef || ses_sessiz != son_sessiz || r != son_r)
		printf("ses: %%%d -> codec %d/7, sessiz %d%s\n", ses_duzey, hedef, ses_sessiz, r ? " (codec yazilamadi)" : "");
	son_r = r;
	son_hedef = hedef;
	son_sessiz = ses_sessiz;
}

static void br_save(void)
{
	struct panel_ayar a = {br_auto, br_manual, tema, ss_on, ss_dk, ses_duzey, ses_sessiz, dnd, ss_tur};
	char yol[160], buf[512];
	int n = panel_ayar_yaz(&a, buf, sizeof buf);

	veri_yol(yol, sizeof yol, "panel.conf");
	if (n > 0 && !dosya_yaz_atomik(yol, buf, (size_t)n))
		sync();
	ses_codec_uygula();	/* 0.39 */
}

/* 0.40: varsayilan tema Kirmizi & Siyah (kullanıcı istegi 19.10). Eski panel.conf'taki tema BIR KEZ 1'e cekilir; isaret
 * dosyasi sayesinde kullanici sonra degistirirse bir daha dokunulmaz. */
static void tema_gocu_040(void)
{
	char yol[160];
	struct stat st;
	veri_yol(yol, sizeof yol, "panel-tema-040");
	if (!stat(yol, &st))
		return;
	int fd = open(yol, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
	if (fd >= 0)
		close(fd);
	if (tema != 1) {
		printf("tema: 0.40 varsayilani Kirmizi & Siyah (bir kez; onceki %d)\n", tema);
		tema = 1;
		br_save();
	}
}

/* PC bagli degilken otomatik mod: 23:00-07:00 %15, diger saatler %80 (saat ayarli degilse gunduz) */
static int br_schedule(void)
{
	time_t now = time(NULL);
	struct tm tm;

	localtime_r(&now, &tm);
	if (tm.tm_year < 120)
		return pct_to_raw(80);
	return (tm.tm_hour >= 23 || tm.tm_hour < 7) ? pct_to_raw(15) : pct_to_raw(80);
}

static void br_apply(void)
{
	int t = br_auto ? (br_pc > 0 ? br_pc : br_schedule()) : pct_to_raw(br_manual);

	if (gece_aktif && t > pct_to_raw(30))	/* 0.41 gece kipi */
		t = pct_to_raw(30);

	if (ss_shown == 1)	/* 0.37: cerceve (2) tam parlaklik */
		t = t / 3 < 3 ? 3 : t / 3;
	else if (ss_shown == 3)	/* 0.40: karartma, en dusuk */
		t = 3;

	if (t != br_cur) {
		br_cur = t;
		disp_brightness(t);
	}
	if (!l_br_auto)
		return;
	if (br_auto)
		lv_label_set_text_fmt(l_br_auto, "Otomatik · şu an %%%d (%s)", raw_to_pct(t),
				      br_pc > 0 ? "PC gece/gündüz ayarı" : "gece 23-07 kısık");
	else
		lv_label_set_text_fmt(l_br_auto, "Manuel: çubukla seç (%%5 - %%100)");
}

static lv_display_t *disp;
static char *last_state;	/* yeniden kurulumdan sonra son PC durumunu tekrar uygulamak icin */
static void ui_build(void);
static void apply_state(const char *line);
static int yeni_sifirla(void);	/* 0.23: yeniden kurulumda yeni nesne isaretcileri (asagida) */
static void rg_temizle(void);	/* 0.24 robot goz (asagida) */
static void ky_temizle(void);	/* 0.29 konusan yuz (asagida) */
static void hv_ciz_cb(lv_event_t *e);	/* 0.32 hava simgesi (asagida) */
static int pv_taze(void);	/* 0.32 cihaz Pano verisi taze mi (asagida) */
static void kb_temizle(void);	/* 0.30 Kole ayar dugmeleri (asagida) */
static void cf_temizle(void);	/* 0.37 cerceve (asagida) */
static void yeni_sonra(int dm_acikti);

static void ui_rebuild_async(void *u)
{
	(void)u;
	int dm_acikti = yeni_sifirla();
	lv_obj_clean(lv_layer_top());
	lv_obj_clean(lv_screen_active());
	macro_sig[0] = prompt_sig[0] = 0;
	l_br_auto = NULL;
	ss_obj = NULL;
	ss_box = NULL;	/* 0.22 */
	ss_priv.kap = NULL;
	rg_temizle();	/* 0.24: nesneler ust katmanla silindi; zamanlayici durur */
	ek_temizle();	/* 0.40 */
	aa_temizle();	/* 0.43: yeniden kurulumda ortu silindi */
	cf_temizle();	/* 0.37 */
	ky_temizle();	/* 0.29 */
	kb_temizle();	/* 0.30 */
	gz_mb = NULL;
	ss_shown = 0;
	wf_win = NULL;
	tm_label = NULL;
	pc_image = l_pc_image = NULL;
	pcs_l_cpu = NULL;	/* 0.67: pcs_build yeniden atar */
	if (pc_full)	/* ekranla birlikte silindi */
		lv_timer_set_period(lv_display_get_refr_timer(disp), LV_DEF_REFR_PERIOD);
	pc_full = pc_full_img = pc_bar = l_pc_bar = NULL;
	br_cur = -1;
	lv_display_set_theme(disp, lv_theme_default_init(disp, C_ACC, lv_color_hex(TH.acc2), TH.dark, f_s));
	ui_build();
	if (last_state)
		apply_state(last_state);
	yeni_sonra(dm_acikti);
	lv_obj_invalidate(lv_screen_active());
	lv_obj_invalidate(lv_layer_top());
	printf("tema: %s\n", TH.ad);
}

static void tema_sec(int t)
{
	if (t < 0 || t >= TEMA_N || t == tema)
		return;
	tema = t;
	br_save();
	lv_async_call(ui_rebuild_async, NULL);
}

static void tema_cb(lv_event_t *e)
{
	tema_sec((int)(uintptr_t)lv_event_get_user_data(e));
}

/* ------------------------------------------------------------ ekran koruyucu */
/* 0.22: yatay duzen [logo | saat / tarih / hava-etkinlik / gizlilik hapi]. Dakikada bir yeni konuma 1.2 sn'de
 * yumusakca kayar (ekran izi kalmasin). Kamera/mikrofon acikken koruyucuda da kirmizi/turuncu hap gorunur. */
#define SS_MS ((uint32_t)ss_dk * 60u * 1000u)	/* 0.23: ayarlanabilir (koruyucu_dk) */
static uint32_t ui_pasif_n;	/* 0.72: koruyucuyu kapatmayan (etkinlik sayilmayan) ui iletisi sayisi */
static lv_obj_t *ss_obj, *ss_clock, *ss_date, *ss_info, *ss_box;
static lv_obj_t *sw_ss;
static hap_t ss_priv;
static char ss_hava[96], ss_etkinlik[128];
static int g_kam, g_mik, g_kayit, g_kalkan;	/* gizlilik durumu, tick_cb her saniye gunceller */
static int32_t ss_x0, ss_y0, ss_x1, ss_y1;
static int ss_last_k = -1;

/* 0.24 robot goz durumu (cizim asagida) */
#define SS_TUR_N PANEL_SS_TUR_N	/* 0.40: 7 tur */
static const char *const ss_tur_ad[SS_TUR_N] = {"Saat", "Robot göz", "Karışık", "Analog saat", "Kayan logo",
						 "Bilgi panosu", "Karartma", "Kelime saati", "Dünya saatleri", "Yıldız alanı", "Aurora"};
static const char *const ss_tur_kod[SS_TUR_N] = {"saat", "robot", "karisik", "analog", "logo", "bilgi", "karanlik",
						  "kelime", "dunya", "yildiz", "aurora"};
static int ss_mod;	/* acik koruyucunun turu */
static rg_robot_t rg;
static const rg_yerlesim_t *rg_yer = &RG_YER_ROBOT;
static lv_obj_t *rg_obj, *rg_saat, *rg_hap_kap;
static hap_t rg_h_kam, rg_h_mik;
static lv_timer_t *rg_tmr;
static uint32_t rg_kapanis;	/* != 0: dokunma tepkisi suruyor, bu anda (ms) kapan */
static rg_poz_t rg_ciz;	/* cizilen (son gecersiz kilinan) poz */
static uint32_t rg_tohum_env;
static int rg_tohum_var, rg_gece_zorla = -1, rg_son_k = -1;
static uint32_t rg_o_kare, rg_o_ciz_n, rg_o_ciz_azami, rg_o_alan_azami, rg_o_t0;	/* olcum (30 sn) */
static uint64_t rg_o_alan, rg_o_ciz_us;
static struct timespec rg_refr_t0;
static void rg_olcum_yaz(uint32_t now, int son);
static void gizle(lv_obj_t *o, int gizli);	/* asagida */

/* LVGL 9.2 ust katman opak olsa da aktif ekrani (sekme/kart/yazi) her yenilemede yine cizer: robot goz acikken
 * ekranin gorunur cocuklari gizlenir, boylece goz alanlari yalniz siyah zemin + goz cizer (C906'da yarim maliyet). */
static lv_obj_t *ss_gizlenen[16];
static int ss_gizlenen_n;

static void ss_alti_gizle(int gizle)
{
	lv_obj_t *scr = lv_screen_active();
	uint32_t n = lv_obj_get_child_count(scr);
	if (gizle) {
		ss_gizlenen_n = 0;
		for (uint32_t i = 0; i < n && ss_gizlenen_n < 16; i++) {
			lv_obj_t *c = lv_obj_get_child(scr, (int32_t)i);
			if (!lv_obj_has_flag(c, LV_OBJ_FLAG_HIDDEN)) {
				lv_obj_add_flag(c, LV_OBJ_FLAG_HIDDEN);
				ss_gizlenen[ss_gizlenen_n++] = c;
			}
		}
		return;
	}
	for (uint32_t i = 0; i < n; i++) {	/* yalniz hala ekranin cocugu olanlar (silinmis isaretci kullanilmaz) */
		lv_obj_t *c = lv_obj_get_child(scr, (int32_t)i);
		for (int k = 0; k < ss_gizlenen_n; k++)
			if (ss_gizlenen[k] == c)
				lv_obj_remove_flag(c, LV_OBJ_FLAG_HIDDEN);
	}
	ss_gizlenen_n = 0;
}

static void rg_temizle(void)
{
	ss_gizlenen_n = 0;	/* yeniden kurulumda ekran cocuklari zaten silindi */
	if (rg_tmr)
		lv_timer_delete(rg_tmr);
	rg_tmr = NULL;
	rg_obj = rg_saat = rg_hap_kap = NULL;
	rg_h_kam.kap = rg_h_mik.kap = NULL;
	rg_kapanis = 0;
}

static void ss_hide(void)
{
	if (rg_obj)
		rg_olcum_yaz(tick_ms(), 1);
	ss_alti_gizle(0);
	rg_temizle();
	ek_temizle();	/* 0.40 */
	if (ss_obj) {
		if (ss_box)	/* 0.24: ss_box NULL iken lv_anim_delete(NULL, NULL) TUM animasyonlari silerdi */
			lv_anim_delete(ss_box, NULL);
		lv_obj_delete(ss_obj);
		ss_obj = NULL;
	}
	cf_temizle();	/* 0.37: nesneler silindikten sonra tamponlar */
	ss_box = NULL;
	ss_priv.kap = NULL;
	ss_shown = 0;
	br_cur = -1;
	br_apply();
}

static void ss_click_cb(lv_event_t *e)
{
	(void)e;
	if (rg_obj && !rg_kapanis) {	/* 0.24: robot goz once dokunulan yere bakip mutlu kirpar, sonra kapanir */
		lv_point_t pt = {W / 2, H / 2};
		lv_indev_t *in = lv_indev_active();
		if (in)
			lv_indev_get_point(in, &pt);
		uint32_t now = tick_ms(), ms = rg_dokun(&rg, pt.x, pt.y, rg_yer, now);
		rg_kapanis = now + ms ? now + ms : 1;
		ss_shown = 0;	/* parlaklik hemen geri gelsin */
		br_cur = -1;
		br_apply();
		lv_display_trigger_activity(disp);
		if (rg_tmr) {
			lv_timer_set_period(rg_tmr, RG_KARE_HIZLI);
			lv_timer_ready(rg_tmr);
		}
		printf("koruyucu: dokunma (%d,%d) -> robot goz tepkisi %u ms\n", (int)pt.x, (int)pt.y, ms);
		return;
	}
	ss_hide();
	lv_display_trigger_activity(disp);
}
static void ss_kay_cb(void *o, int32_t v)
{
	int32_t x = ss_x0 + (ss_x1 - ss_x0) * v / 1024, y = ss_y0 + (ss_y1 - ss_y0) * v / 1024;
	lv_obj_align((lv_obj_t *)o, LV_ALIGN_CENTER, x, y);
}

static void ss_priv_guncelle(void)
{
	if (!ss_priv.kap)
		return;
	if (g_kam || g_mik) {
		char t[64];
		snprintf(t, sizeof t, "%s%s%s", g_kam ? "KAMERA AÇIK" : "", g_kam && g_mik ? " · " : "", g_mik ? "MİKROFON AÇIK" : "");
		hap_ayarla(&ss_priv, lv_color_hex(g_kam ? 0xDC2626 : 0xEA580C), 1, t);
		lv_obj_remove_flag(ss_priv.kap, LV_OBJ_FLAG_HIDDEN);
	} else
		lv_obj_add_flag(ss_priv.kap, LV_OBJ_FLAG_HIDDEN);
}

static void ss_update(void)
{
	time_t now = time(NULL);
	struct tm tm;

	if (!ss_obj)
		return;
	localtime_r(&now, &tm);
	lv_label_set_text_fmt(ss_clock, "%02d:%02d", tm.tm_hour, tm.tm_min);
	if (tm.tm_year > 120)
		lv_label_set_text_fmt(ss_date, "%d %s %s", tm.tm_mday, aylar[tm.tm_mon], gunler[tm.tm_wday]);
	char info[240];
	snprintf(info, sizeof info, "%s%s%s", ss_hava, ss_hava[0] && ss_etkinlik[0] ? "    ·    " : "", ss_etkinlik);
	if (strcmp(lv_label_get_text(ss_info), info))
		lv_label_set_text(ss_info, info);
	ss_priv_guncelle();
	/* ekran izi kalmasin: her dakika farkli konum, yumusak gecis */
	int k = tm.tm_min % 8;
	if (k != ss_last_k) {
		int ilk = ss_last_k < 0;
		ss_last_k = k;
		int32_t x = (int32_t)((k % 4) * 140 - 210), y = (k / 4 ? 1 : -1) * 60;
		if (ilk || lv_anim_get(ss_box, ss_kay_cb)) {
			ss_x1 = x;
			ss_y1 = y;
			lv_obj_align(ss_box, LV_ALIGN_CENTER, x, y);
		} else {
			ss_x0 = ss_x1;
			ss_y0 = ss_y1;
			ss_x1 = x;
			ss_y1 = y;
			lv_anim_t a;
			lv_anim_init(&a);
			lv_anim_set_var(&a, ss_box);
			lv_anim_set_exec_cb(&a, ss_kay_cb);
			lv_anim_set_values(&a, 0, 1024);
			lv_anim_set_duration(&a, 1200);
			lv_anim_set_path_cb(&a, lv_anim_path_ease_in_out);
			lv_anim_start(&a);
		}
	}
}

static void ss_show_saat(void)
{
	dm_git(0, 0);	/* 0.23 */
	ss_mod = 0;
	ss_obj = lv_obj_create(lv_layer_top());
	lv_obj_remove_style_all(ss_obj);
	lv_obj_set_size(ss_obj, W, H);
	lv_obj_set_style_bg_opa(ss_obj, LV_OPA_COVER, 0);
	lv_obj_set_style_bg_color(ss_obj, lv_color_black(), 0);
	lv_obj_add_flag(ss_obj, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_add_event_cb(ss_obj, ss_click_cb, LV_EVENT_PRESSED, NULL);
	ss_box = lv_obj_create(ss_obj);
	lv_obj_remove_style_all(ss_box);
	lv_obj_set_size(ss_box, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
	lv_obj_set_flex_flow(ss_box, LV_FLEX_FLOW_ROW);
	lv_obj_set_flex_align(ss_box, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	lv_obj_set_style_pad_column(ss_box, 44, 0);
	lv_obj_remove_flag(ss_box, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_t *ssl = lv_image_create(ss_box);
	lv_image_set_src(ssl, &logo_tam);
	lv_obj_set_style_image_opa(ssl, LV_OPA_40, 0);
	lv_image_set_scale(ssl, 160);	/* 256 = %100 */
	lv_obj_set_size(ssl, logo_tam.header.w * 160 / 256, logo_tam.header.h * 160 / 256);
	lv_image_set_inner_align(ssl, LV_IMAGE_ALIGN_CENTER);
	lv_obj_t *col = lv_obj_create(ss_box);
	lv_obj_remove_style_all(col);
	lv_obj_set_size(col, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
	lv_obj_set_flex_flow(col, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_style_pad_row(col, 4, 0);
	lv_obj_remove_flag(col, LV_OBJ_FLAG_CLICKABLE);
	ss_clock = label(col, f_clock, "");
	lv_obj_set_style_text_color(ss_clock, lv_color_hex(0xA1A1AA), 0);
	ss_date = label(col, f_l, "");
	lv_obj_set_style_text_color(ss_date, lv_color_hex(0x71717A), 0);
	ss_info = label(col, f_m, "");
	lv_obj_set_width(ss_info, 880);	/* kutu genisligi sabit: kayma ekrandan tasmasin */
	lv_obj_set_height(ss_info, lv_font_get_line_height(f_m));	/* tek satir: sigmazsa "..." */
	lv_label_set_long_mode(ss_info, LV_LABEL_LONG_DOT);
	lv_obj_set_style_text_color(ss_info, lv_color_hex(0x52525B), 0);
	lv_obj_set_style_pad_top(ss_info, 6, 0);
	ss_priv = hap(col);
	lv_obj_set_style_margin_top(ss_priv.kap, 12, 0);
	lv_obj_add_flag(ss_priv.kap, LV_OBJ_FLAG_HIDDEN);
	ss_shown = 1;
	ss_last_k = -1;
	ss_update();
	br_cur = -1;
	br_apply();
}

/* ------------------------------------------------------- 0.24 robot goz cizimi */
typedef struct { uint32_t govde, parilti, bebek; } rg_renk_t;

static rg_renk_t rg_renk_sec(uint8_t renk)
{
	/* koruyucu zemini her temada siyah; goz rengi temaya gore. Kamera acikken (izleniyor) her temada kirmizi. */
	static const rg_renk_t tema_r[3] = {{0x22D3EE, 0x06B6D4, 0x05262E},	/* Mavi: camgobegi parilti */
					    {0xFFE4E6, 0xE11D48, 0x3B0A14},	/* Kirmizi & Siyah: beyaz goz, kirmizi hale */
					    {0x7DD3FC, 0x3B82F6, 0x0B2540}};	/* Beyaz: acik mavi */
	static const rg_renk_t kirmizi = {0xFF3B3B, 0xFF0000, 0x380505};
	if (renk == RG_R_KIRMIZI)
		return kirmizi;
	return tema_r[tema >= 0 && tema < 3 ? tema : 0];
}

static void rg_dortgen(lv_layer_t *l, int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t r, lv_color_t c, lv_opa_t opa)
{
	lv_draw_rect_dsc_t d;
	lv_area_t a = {x1, y1, x2, y2};
	lv_draw_rect_dsc_init(&d);
	d.bg_color = c;
	d.bg_opa = opa;
	d.radius = r;
	lv_draw_rect(l, &d, &a);
}

static void rg_ucgen(lv_layer_t *l, int32_t ax, int32_t ay, int32_t bx, int32_t by, int32_t cx, int32_t cy, lv_color_t c)
{
	lv_draw_triangle_dsc_t t;
	lv_draw_triangle_dsc_init(&t);
	t.bg_color = c;
	t.bg_opa = LV_OPA_COVER;
	t.p[0].x = ax;
	t.p[0].y = ay;
	t.p[1].x = bx;
	t.p[1].y = by;
	t.p[2].x = cx;
	t.p[2].y = cy;
	lv_draw_triangle(l, &t);
}

static void rg_yay(lv_layer_t *l, int32_t cx, int32_t cy, int32_t r, int32_t w, int32_t a1, int32_t a2, lv_color_t c)
{
	lv_draw_arc_dsc_t d;
	lv_draw_arc_dsc_init(&d);
	d.color = c;
	d.width = w;
	d.start_angle = a1;
	d.end_angle = a2;
	d.center.x = cx;
	d.center.y = cy;
	d.radius = (uint16_t)r;
	d.rounded = 1;
	d.opa = LV_OPA_COVER;
	lv_draw_arc(l, &d);
}

static void rg_ciz_normal(lv_layer_t *l, const rg_goz_t *g, const rg_renk_t *rk)
{
	lv_color_t siyah = lv_color_black();
	if (g->parilti) {	/* ucuz parilti: iki ince yari saydam halka (yalniz kenar pikselleri karisir) */
		lv_draw_rect_dsc_t d;
		lv_draw_rect_dsc_init(&d);
		d.bg_opa = LV_OPA_TRANSP;
		d.border_color = lv_color_hex(rk->parilti);
		d.border_width = RG_PARILTI / 2;
		for (int i = 2; i >= 1; i--) {
			int32_t m = RG_PARILTI * i / 2;
			lv_area_t h = {g->x1 - m, g->y1 - m, g->x2 + m, g->y2 + m};
			d.border_opa = i == 2 ? 34 : 78;
			d.radius = g->r + m;
			lv_draw_rect(l, &d, &h);
		}
	}
	rg_dortgen(l, g->x1, g->y1, g->x2, g->y2, g->r, lv_color_hex(rk->govde), LV_OPA_COVER);
	if (g->pr) {
		rg_dortgen(l, g->pcx - g->pr, g->pcy - g->pr, g->pcx + g->pr, g->pcy + g->pr, LV_RADIUS_CIRCLE,
			   lv_color_hex(rk->bebek), LV_OPA_COVER);
		rg_dortgen(l, g->hx - g->hr, g->hy - g->hr, g->hx + g->hr, g->hy + g->hr, LV_RADIUS_CIRCLE, lv_color_white(), 235);
		int32_t k = g->hr / 2;	/* ikinci kucuk parilti */
		if (k >= 3)
			rg_dortgen(l, g->pcx + g->pr * 30 / 100 - k, g->pcy + g->pr * 34 / 100 - k, g->pcx + g->pr * 30 / 100 + k,
				   g->pcy + g->pr * 34 / 100 + k, LV_RADIUS_CIRCLE, lv_color_white(), 150);
	}
	int32_t m = RG_PARILTI + 3;
	if (g->ust_ic_y > g->y1 || g->ust_dis_y > g->y1) {	/* egik ust kapak: siyah dortgen (iki ucgen) */
		int32_t xi = g->ic_x, xd = g->dis_x, yi = g->ust_ic_y, yd = g->ust_dis_y, yon = xi > xd ? 1 : -1;
		int32_t dx = xi - xd ? xi - xd : 1;
		int32_t xi2 = xi + yon * m, xd2 = xd - yon * m;
		int32_t yi2 = yd + (yi - yd) * (xi2 - xd) / dx, yd2 = yd + (yi - yd) * (xd2 - xd) / dx;
		int32_t ust = g->y1 - m;
		/* LVGL 9.2 sw ucgeni: dikey kenarli ucgende ucuncu nokta en altta degilse alt kenar maskesi ters taraf
		 * secer (provada kapak ustunde kama kaldi). Dikey/yatay kenar birakmamak icin 1 px kaydirilmis noktalar;
		 * ikinci ucgen ilkiyle B-L2 boyunca biraz ortusur (sac teli kadar dikis kalmasin). */
		rg_ucgen(l, xd2, yd2, xi2, ust + 1, xi2 + yon, yi2, siyah);
		rg_ucgen(l, xd2, ust, xi2, ust + 1, xd2 - yon, yd2 + 2, siyah);
	}
	if (g->alt_y < g->y2)
		rg_dortgen(l, g->x1 - m, g->alt_y, g->x2 + m, g->y2 + m, 0, siyah, LV_OPA_COVER);
}

static void rg_ciz_cb(lv_event_t *e)
{
	lv_layer_t *l = lv_event_get_layer(e);
	const rg_poz_t *p = &rg_ciz;
	rg_renk_t rk = rg_renk_sec(p->renk);
	lv_color_t cg = lv_color_hex(rk.govde);
	lv_area_t kirp = l->_clip_area;

	for (int s = 0; s < 2; s++) {
		rg_goz_t g;
		rg_goz_hesapla(p, s, rg_yer, &g);
		if (g.kx2 < kirp.x1 || g.kx1 > kirp.x2 || g.ky2 < kirp.y1 || g.ky1 > kirp.y2)
			continue;	/* bu goz yeniden cizilen alanda degil */
		int32_t R = g.olc;
		switch (g.sekil) {
		case RG_S_NORMAL:
			rg_ciz_normal(l, &g, &rk);
			break;
		case RG_S_MUTLU:	/* ^ */
			rg_yay(l, g.cx, g.cy, R, R * 34 / 100 > 18 ? R * 34 / 100 : 18, 200, 340, cg);
			break;
		case RG_S_KAPALI:	/* uyuyan goz: asagi kavis */
			rg_yay(l, g.cx, g.cy, R, R * 14 / 100 > 12 ? R * 14 / 100 : 12, 22, 158, cg);
			break;
		default: {	/* kalp: iki daire + ucgen */
			lv_color_t pembe = lv_color_hex(0xFF4D8D);
			int32_t q = R * 27 / 100, ox = R * 23 / 100, oy = R * 12 / 100;
			rg_dortgen(l, g.cx - ox - q, g.cy - oy - q, g.cx - ox + q, g.cy - oy + q, LV_RADIUS_CIRCLE, pembe, LV_OPA_COVER);
			rg_dortgen(l, g.cx + ox - q, g.cy - oy - q, g.cx + ox + q, g.cy - oy + q, LV_RADIUS_CIRCLE, pembe, LV_OPA_COVER);
			rg_ucgen(l, g.cx - ox - q, g.cy - oy, g.cx + ox + q, g.cy - oy + 1, g.cx, g.cy + R * 55 / 100, pembe);
			break;
		}
		}
	}
	if (p->zzz) {
		static const char *const zt[3] = {"z", "z", "Z"};
		const lv_font_t *zf[3] = {f_m, f_l, f_xl};
		for (int i = 0; i < 3; i++) {
			int zx, zy;
			if (!rg_zzz_harf(p, rg_yer, i, &zx, &zy))
				continue;
			lv_draw_label_dsc_t d;
			lv_draw_label_dsc_init(&d);
			d.text = zt[i];
			d.font = zf[i];
			d.color = cg;
			int yas = p->zzz - 1 - i * 3;
			d.opa = (lv_opa_t)(240 - yas * 16);
			lv_area_t a = {zx, zy, zx + 50, zy + 75};
			lv_draw_label(l, &d, &a);
		}
	}
}

/* (eski U yeni) kutular gecersiz; donus: alan (px) */
static uint32_t rg_gecersiz(const rg_poz_t *a, const rg_poz_t *b)
{
	uint32_t alan = 0;
	for (int s = 0; s < 2; s++) {
		rg_goz_t ga, gb;
		int16_t x1, y1, x2, y2;
		rg_goz_hesapla(a, s, rg_yer, &ga);
		rg_goz_hesapla(b, s, rg_yer, &gb);
		alan += (uint32_t)rg_birlesim(&ga, &gb, &x1, &y1, &x2, &y2);
		lv_area_t ar = {x1, y1, x2, y2};
		lv_obj_invalidate_area(rg_obj, &ar);
	}
	return alan;
}

static void rg_olcum_yaz(uint32_t now, int son)
{
	uint32_t sure = now - rg_o_t0;
	if (!son && sure < 30000)
		return;
	/* 0.79: 30 sn'lik olcum satiri silindi (0.24 olcum araci; panel.log'u buyutuyordu, denetim v3 A13) */
	rg_o_t0 = now;
	rg_o_kare = rg_o_ciz_n = rg_o_ciz_azami = rg_o_alan_azami = 0;
	rg_o_alan = rg_o_ciz_us = 0;
}

/* yenileme suresi (REFR_START -> RENDER_READY: duzen + cizim + flush), yalniz robot goz acikken */
static void rg_refr_cb(lv_event_t *e)
{
	if (!rg_obj)
		return;
	if (lv_event_get_code(e) == LV_EVENT_REFR_START)
		clock_gettime(CLOCK_MONOTONIC, &rg_refr_t0);
	else {
		struct timespec t1;
		clock_gettime(CLOCK_MONOTONIC, &t1);
		uint32_t us = (uint32_t)elapsed_us(&rg_refr_t0, &t1);
		rg_o_ciz_n++;
		rg_o_ciz_us += us;
		if (us > rg_o_ciz_azami)
			rg_o_ciz_azami = us;
	}
}

static void rg_tmr_cb(lv_timer_t *t)
{
	uint32_t now = tick_ms();
	if (!rg_obj)
		return;
	if (rg_kapanis && (int32_t)(now - rg_kapanis) >= 0) {
		printf("koruyucu: kapandi (dokunma)\n");
		ss_hide();	/* zamanlayici burada silinir (LVGL kendi geri cagrisinda silmeye izin verir) */
		lv_display_trigger_activity(disp);
		return;
	}
	if (rg_adim(&rg, now)) {
		uint32_t alan = rg_gecersiz(&rg_ciz, &rg.simdi);
		rg_ciz = rg.simdi;
		rg_o_kare++;
		rg_o_alan += alan;
		if (alan > rg_o_alan_azami)
			rg_o_alan_azami = alan;
	}
	uint32_t d = rg_sonraki_ms(&rg, now);
	if (rg_kapanis) {
		uint32_t k = rg_kapanis - now;
		if (k < d)
			d = k ? k : 1;
	}
	lv_timer_set_period(t, d);
	rg_olcum_yaz(now, 0);
}

/* saniyede bir (ss_tick): girdiler, haplar, taban kaymasi, karisik modda saat satiri */
static void rg_saniye(void)
{
	time_t now = time(NULL);
	struct tm tm;
	localtime_r(&now, &tm);
	int saat_var = tm.tm_year > 120;
	int gece = rg_gece_zorla >= 0 ? rg_gece_zorla : saat_var && (tm.tm_hour >= 23 || tm.tm_hour < 7);
	rg_girdi(&rg, g_kam, g_mik, gece, tick_ms());
	if (rg_h_kam.kap) {	/* gizlilik hapi: koruyucuda da HER ZAMAN gorunur */
		if (g_kam)
			hap_ayarla(&rg_h_kam, lv_color_hex(0xDC2626), 1, LV_SYMBOL_EYE_OPEN "  KAMERA AÇIK");
		gizle(rg_h_kam.kap, !g_kam);
		if (g_mik)
			hap_ayarla(&rg_h_mik, lv_color_hex(0xEA580C), 1, LV_SYMBOL_AUDIO "  MİKROFON");
		gizle(rg_h_mik.kap, !g_mik);
	}
	int k = saat_var ? tm.tm_min % 8 : 0;	/* ekran izi: dakikada bir taban kayar */
	if (k != rg_son_k) {
		static const int16_t dx[4] = {-80, -27, 27, 80};
		rg_son_k = k;
		rg_taban(&rg, dx[k % 4], k / 4 ? 14 : -14);
		if (rg_hap_kap)
			lv_obj_align(rg_hap_kap, LV_ALIGN_TOP_MID, dx[k % 4] / 2, 20);
		if (rg_saat)
			lv_obj_align(rg_saat, LV_ALIGN_BOTTOM_MID, dx[k % 4], -26);
	}
	if (rg_saat) {
		char b[240];
		if (saat_var)
			snprintf(b, sizeof b, "%02d:%02d   ·   %d %s %s%s%s", tm.tm_hour, tm.tm_min, tm.tm_mday, aylar[tm.tm_mon],
				 gunler[tm.tm_wday], ss_hava[0] ? "   ·   " : "", ss_hava);
		else
			snprintf(b, sizeof b, "%s", ss_hava[0] ? ss_hava : "Saat ayarlanmadı");
		if (strcmp(lv_label_get_text(rg_saat), b))
			lv_label_set_text(rg_saat, b);
	}
}

static void rg_bildirim_olay(void)
{
	if (rg_obj && !rg_kapanis) {
		rg_bildirim(&rg, tick_ms());
		if (rg_tmr)
			lv_timer_ready(rg_tmr);
	}
}

static void ek_goster(int tur);	/* 0.40 (asagida) */
static void ss_show_tur(int tur)
{
	if (tur >= 3 && tur < SS_TUR_N) {	/* 0.40 */
		ek_goster(tur);
		return;
	}
	if (tur < 1 || tur > 2) {
		ss_show_saat();
		printf("koruyucu: saat\n");
		return;
	}
	dm_git(0, 0);
	ss_mod = tur;
	rg_yer = tur == 2 ? &RG_YER_KARISIK : &RG_YER_ROBOT;
	ss_obj = lv_obj_create(lv_layer_top());
	lv_obj_remove_style_all(ss_obj);
	lv_obj_set_size(ss_obj, W, H);
	lv_obj_set_style_bg_opa(ss_obj, LV_OPA_COVER, 0);
	lv_obj_set_style_bg_color(ss_obj, lv_color_black(), 0);
	lv_obj_remove_flag(ss_obj, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_add_flag(ss_obj, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_add_event_cb(ss_obj, ss_click_cb, LV_EVENT_PRESSED, NULL);
	rg_obj = lv_obj_create(ss_obj);	/* gozler: tek nesne, DRAW_MAIN'de ilkellerle cizilir */
	lv_obj_remove_style_all(rg_obj);
	lv_obj_set_size(rg_obj, W, H);
	lv_obj_remove_flag(rg_obj, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_add_event_cb(rg_obj, rg_ciz_cb, LV_EVENT_DRAW_MAIN, NULL);
	rg_hap_kap = lv_obj_create(ss_obj);
	lv_obj_remove_style_all(rg_hap_kap);
	lv_obj_set_size(rg_hap_kap, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
	lv_obj_set_flex_flow(rg_hap_kap, LV_FLEX_FLOW_ROW);
	lv_obj_set_style_pad_column(rg_hap_kap, 12, 0);
	lv_obj_remove_flag(rg_hap_kap, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
	rg_h_kam = hap(rg_hap_kap);
	rg_h_mik = hap(rg_hap_kap);
	lv_obj_add_flag(rg_h_kam.kap, LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_flag(rg_h_mik.kap, LV_OBJ_FLAG_HIDDEN);
	if (tur == 2) {
		rg_saat = label(ss_obj, f_m, "");
		lv_obj_set_style_text_color(rg_saat, lv_color_hex(0x71717A), 0);
	}
	uint32_t now = tick_ms();
	rg_baslat(&rg, rg_tohum_var ? rg_tohum_env : (now * 2654435761u) ^ (uint32_t)time(NULL) ^ ((uint32_t)getpid() << 16),
		  now);
	rg_son_k = -1;
	rg_kapanis = 0;
	rg_saniye();
	rg_adim(&rg, now);
	rg_ciz = rg.simdi;
	rg_tmr = lv_timer_create(rg_tmr_cb, RG_KARE_HIZLI, NULL);
	ss_alti_gizle(1);
	rg_o_t0 = now;
	rg_o_kare = rg_o_ciz_n = rg_o_ciz_azami = rg_o_alan_azami = 0;
	rg_o_alan = rg_o_ciz_us = 0;
	ss_shown = 1;
	ss_last_k = -1;
	br_cur = -1;
	br_apply();
	printf("koruyucu: %s (tohum %s)\n", ss_tur_kod[tur], rg_tohum_var ? "sabit" : "rastgele");
}

/* ------------------------------------------------------------ 0.40 ek koruyucu turleri */
/* 3 Analog saat (tek nesne DRAW_MAIN: kadran + akrep/yelkovan/saniye; saniyede bir yalniz kadran alani gecersiz),
 * 4 Kayan logo (logo + saat kutusu ekranda seker, 80 ms adim), 5 Bilgi panosu (saat, tarih, hava, doviz, etkinlik,
 * baglanti), 6 Karartma (siyah, kucuk soluk saat, parlaklik en dusuk: ss_shown 3). Hepsi siyah zemin; alttaki arayuz
 * gizlenir (ss_alti_gizle, robot goz gibi); dakikada bir yer degistirir; kamera/mikrofon hapi gorunur. ss_mod 10+tur. */
static char g_ip[32];	/* asagida tanimli (ilk deger orada) */
static int g_pc_ok;
static lv_obj_t *ek_kutu, *ek_kadran, *ek_logo, *ek_l1, *ek_l2, *ek_l3, *ek_l4, *ek_l5;
static lv_timer_t *ek_tmr;
static int ek_tur, ek_renk_i;
static int32_t ek_x, ek_y, ek_dx = 2, ek_dy = 2;
#define EK_R 200	/* analog kadran yaricapi */
/* 0.44 ek koruyucular: kelime saati, dunya saatleri, yildiz alani, aurora */
static void ek_metin(lv_obj_t *o, const char *t);	/* asagida */
static lv_obj_t *ek_yildiz, *dz_l[4][3];
#define YZ_N 70
static float yz_x[YZ_N], yz_y[YZ_N], yz_z[YZ_N];
static uint32_t yz_rng = 0x2468ace1u;

static const char *const ks_yalin[13] = {"", "bir", "iki", "üç", "dört", "beş", "altı", "yedi", "sekiz", "dokuz", "on",
					 "on bir", "on iki"};
static const char *const ks_bel[13] = {"", "biri", "ikiyi", "üçü", "dördü", "beşi", "altıyı", "yediyi", "sekizi", "dokuzu",
				       "onu", "on biri", "on ikiyi"};
static const char *const ks_yon[13] = {"", "bire", "ikiye", "üçe", "dörde", "beşe", "altıya", "yediye", "sekize", "dokuza",
				       "ona", "on bire", "on ikiye"};
static const char *const ks_dk[6] = {"", "beş", "on", "çeyrek", "yirmi", "yirmi beş"};

/* Turkce sozcuklerle saat, en yakin 5 dakika: ust satir ("dokuzu"/"ona"/"saat"), alt satir ("çeyrek geçiyor") */
static void kelime_saat(int h24, int m, char *a, size_t na, char *b, size_t nb)
{
	int m5 = (m + 2) / 5 * 5, h = h24 % 12;
	if (m5 == 60) {
		m5 = 0;
		h = (h + 1) % 12;
	}
	int s = h ? h : 12, n = s % 12 + 1;
	if (m5 == 0) {
		snprintf(a, na, "saat");
		snprintf(b, nb, "%s", ks_yalin[s]);
	} else if (m5 == 30) {
		snprintf(a, na, "saat");
		snprintf(b, nb, "%s buçuk", ks_yalin[s]);
	} else if (m5 < 30) {
		snprintf(a, na, "%s", ks_bel[s]);
		snprintf(b, nb, "%s geçiyor", ks_dk[m5 / 5]);
	} else {
		snprintf(a, na, "%s", ks_yon[n]);
		snprintf(b, nb, "%s var", ks_dk[(60 - m5) / 5]);
	}
}

/* yil/ay (1..12) icinde n. pazar (n=-1: son pazar) gunu, UTC 00:00 */
static time_t dz_pazar(int yil, int ay, int n)
{
	struct tm t = {0};
	t.tm_year = yil - 1900;
	t.tm_mon = ay - 1;
	t.tm_mday = 1;
	time_t ilk = timegm(&t);
	struct tm g;
	gmtime_r(&ilk, &g);
	int ilk_pazar = 1 + (7 - g.tm_wday) % 7;
	if (n > 0)
		return ilk + (time_t)(ilk_pazar - 1 + 7 * (n - 1)) * 86400;
	int gun = ilk_pazar;
	while (gun + 7 <= (ay == 2 ? 28 : (ay == 4 || ay == 6 || ay == 9 || ay == 11) ? 30 : 31))
		gun += 7;
	return ilk + (time_t)(gun - 1) * 86400;
}

/* UTC'ye gore saat farki (sn): 0 Istanbul, 1 Londra, 2 New York, 3 Tokyo */
static long dz_fark(int i, time_t u)
{
	struct tm g;
	gmtime_r(&u, &g);
	int y = g.tm_year + 1900;
	if (i == 0)
		return 3 * 3600;
	if (i == 3)
		return 9 * 3600;
	if (i == 1) {	/* AB: son pazar mart 01:00 UTC .. son pazar ekim 01:00 UTC */
		int yaz = u >= dz_pazar(y, 3, -1) + 3600 && u < dz_pazar(y, 10, -1) + 3600;
		return yaz ? 3600 : 0;
	}
	/* ABD: mart 2. pazar 07:00 UTC .. kasim 1. pazar 06:00 UTC */
	int yaz = u >= dz_pazar(y, 3, 2) + 7 * 3600 && u < dz_pazar(y, 11, 1) + 6 * 3600;
	return yaz ? -4 * 3600 : -5 * 3600;
}

static void dz_guncelle(void)
{
	static const char *const ad[4] = {"İstanbul", "Londra", "New York", "Tokyo"};
	time_t u = time(NULL);
	struct tm yer;
	localtime_r(&u, &yer);
	int bugun = yer.tm_yday + yer.tm_year * 400;
	char b[32];
	for (int i = 0; i < 4; i++) {
		if (!dz_l[i][0])
			continue;
		time_t t = u + dz_fark(i, u);
		struct tm g;
		gmtime_r(&t, &g);
		int gun = g.tm_yday + g.tm_year * 400;
		ek_metin(dz_l[i][0], ad[i]);
		snprintf(b, sizeof b, "%02d:%02d", g.tm_hour, g.tm_min);
		ek_metin(dz_l[i][1], b);
		ek_metin(dz_l[i][2], gun > bugun ? "yarın" : gun < bugun ? "dün" : "bugün");
	}
}

static float yz_rast(void)
{
	yz_rng = yz_rng * 1664525u + 1013904223u;
	return (float)(yz_rng >> 8) / 16777216.0f;
}

static void yz_yeni(int i, float z)
{
	yz_x[i] = yz_rast() * 2.0f - 1.0f;
	yz_y[i] = yz_rast() * 2.0f - 1.0f;
	yz_z[i] = z;
}

/* 0.62: kirpma (PANEL_YZ_KIRPMA=0 kapatir), dondurma (PANEL_YZ_TEST=dondur) ve kare maliyeti olcumu */
static int yz_kirpma = 1, yz_dondur;
static uint32_t yz_o_t0, yz_o_kare, yz_o_azami;
static uint64_t yz_o_us;
static unsigned long long yz_o_g2d0, yz_o_g2dus0;
static struct timespec yz_refr_t0;

static void yz_ciz_cb(lv_event_t *e)
{
	lv_layer_t *l = lv_event_get_layer(e);
	lv_area_t kirp = l->_clip_area;	/* 0.62: kismi kipte her serit (1600x60) icin cagrilir */
	for (int i = 0; i < YZ_N; i++) {
		float z = yz_z[i];
		int32_t x = (int32_t)(W / 2 + yz_x[i] / z * 420.0f), y = (int32_t)(H / 2 + yz_y[i] / z * 260.0f);
		if (x < 0 || x >= W || y < 0 || y >= H)
			continue;
		int32_t r = (int32_t)(1.0f + (1.0f - z) * 3.0f);
		if (yz_kirpma && (x + r < kirp.x1 || x - r > kirp.x2 || y + r < kirp.y1 || y - r > kirp.y2))
			continue;	/* 0.62: bu seritte gorunmez; LVGL kirpma disina zaten cizmez (piksel ayni) */
		lv_opa_t opa = (lv_opa_t)(60 + (1.0f - z) * 195.0f);
		rg_dortgen(l, x - r, y - r, x + r, y + r, r, i % 9 == 0 ? C_ACC : lv_color_hex(0xE7E5E4), opa);
	}
}

/* 0.62: yildiz alani acikken yenileme suresi (REFR_START -> RENDER_READY: cizim + tum seritlerin aktarimi) */
static void yz_refr_cb(lv_event_t *e)
{
	if (!ek_yildiz)
		return;
	if (lv_event_get_code(e) == LV_EVENT_REFR_START) {
		clock_gettime(CLOCK_MONOTONIC, &yz_refr_t0);
		return;
	}
	struct timespec t1;
	clock_gettime(CLOCK_MONOTONIC, &t1);
	uint32_t us = (uint32_t)elapsed_us(&yz_refr_t0, &t1);
	yz_o_kare++;
	yz_o_us += us;
	if (us > yz_o_azami)
		yz_o_azami = us;
}

static void yz_olcum_yaz(uint32_t now)
{
	uint32_t sure = now - yz_o_t0;
	if (sure < 60000)
		return;
	unsigned long long g = pg_say - yz_o_g2d0, gu = pg_us - yz_o_g2dus0;
	printf("yildiz: %u sn, kare %u (%u.%u/sn), yenileme ort %u us azami %u us, g2d %llu/%llu us%s\n", sure / 1000, yz_o_kare,
	       yz_o_kare * 1000 / sure, yz_o_kare * 10000 / sure % 10, yz_o_kare ? (uint32_t)(yz_o_us / yz_o_kare) : 0u,
	       yz_o_azami, g, g ? gu / g : 0ULL, yz_kirpma ? "" : " (kirpma kapali)");
	yz_o_t0 = now;
	yz_o_kare = yz_o_azami = 0;
	yz_o_us = 0;
	yz_o_g2d0 = pg_say;
	yz_o_g2dus0 = pg_us;
}

static void yz_tmr_cb(lv_timer_t *t)
{
	(void)t;
	if (!ek_yildiz)
		return;
	yz_olcum_yaz(tick_ms());
	if (yz_dondur) {	/* 0.62 sinama: hareket yok, yine her tikte yeniden cizim */
		lv_obj_invalidate(ek_yildiz);
		return;
	}
	for (int i = 0; i < YZ_N; i++) {
		yz_z[i] -= 0.012f;
		float z = yz_z[i];
		if (z < 0.06f || fabsf(yz_x[i] / z * 420.0f) > W / 2 || fabsf(yz_y[i] / z * 260.0f) > H / 2)
			yz_yeni(i, 1.0f);
	}
	lv_obj_invalidate(ek_yildiz);
}

/* aurora: 120 sn'lik dongude dort koyu renk arasinda; ust ve alt renk 30 sn faz farkli */
static lv_color_t au_renk(uint32_t sn)
{
	static const uint32_t p[4] = {0x2A0614, 0x0B1238, 0x042A26, 0x2A1004};
	uint32_t f = sn % 120, i = f / 30, k = (f % 30) * 255 / 30;
	return lv_color_mix(lv_color_hex(p[(i + 1) % 4]), lv_color_hex(p[i]), (uint8_t)k);
}

static void au_guncelle(void)
{
	if (!ss_obj)
		return;
	uint32_t sn = (uint32_t)time(NULL);
	lv_obj_set_style_bg_color(ss_obj, au_renk(sn), 0);
	lv_obj_set_style_bg_grad_color(ss_obj, au_renk(sn + 30), 0);
}

static void ek_temizle(void)
{
	if (ek_kutu)
		lv_anim_delete(ek_kutu, NULL);
	if (ek_tmr)
		lv_timer_delete(ek_tmr);
	ek_tmr = NULL;
	ek_kutu = ek_kadran = ek_logo = ek_l1 = ek_l2 = ek_l3 = ek_l4 = ek_l5 = NULL;
	ek_yildiz = NULL;	/* 0.44 */
	memset(dz_l, 0, sizeof dz_l);
	ek_tur = 0;
}

static void ek_cizgi(lv_layer_t *l, double x1, double y1, double x2, double y2, int32_t w, lv_color_t c)
{
	lv_draw_line_dsc_t d;
	lv_draw_line_dsc_init(&d);
	d.p1.x = (int32_t)lrint(x1);
	d.p1.y = (int32_t)lrint(y1);
	d.p2.x = (int32_t)lrint(x2);
	d.p2.y = (int32_t)lrint(y2);
	d.color = c;
	d.width = w;
	d.opa = LV_OPA_COVER;
	d.round_start = d.round_end = 1;
	lv_draw_line(l, &d);
}

/* a: radyan (0 = saat 12, saat yonunde); kuyruk: merkezin arkasina uzanan kisim */
static void ek_ibre(lv_layer_t *l, int32_t cx, int32_t cy, double a, int32_t boy, int32_t kuyruk, int32_t w,
		    lv_color_t c)
{
	ek_cizgi(l, cx - kuyruk * sin(a), cy + kuyruk * cos(a), cx + boy * sin(a), cy - boy * cos(a), w, c);
}

static void ek_kadran_cb(lv_event_t *e)
{
	lv_obj_t *o = lv_event_get_target(e);
	lv_layer_t *l = lv_event_get_layer(e);
	lv_area_t a;
	lv_obj_get_coords(o, &a);
	int32_t cx = (a.x1 + a.x2) / 2, cy = (a.y1 + a.y2) / 2;
	time_t now = time(NULL);
	struct tm tm;
	localtime_r(&now, &tm);
	lv_color_t acc = C_ACC, ak = lv_color_hex(0xE7E5E4), soluk = lv_color_hex(0x3F3F46);
	rg_yay(l, cx, cy, EK_R, 5, 0, 360, lv_color_hex(0x27272A));
	for (int i = 0; i < 60; i++) {
		double t = i * M_PI / 30;
		int b = i % 5 == 0;
		double r1 = EK_R - (b ? 36 : 18), r2 = EK_R - 14;
		ek_cizgi(l, cx + r1 * sin(t), cy - r1 * cos(t), cx + r2 * sin(t), cy - r2 * cos(t), b ? 7 : 3,
			 b ? (i % 15 == 0 ? acc : ak) : soluk);
	}
	double sa = ((tm.tm_hour % 12) + tm.tm_min / 60.0) * M_PI / 6;
	double dk = (tm.tm_min + tm.tm_sec / 60.0) * M_PI / 30;
	ek_ibre(l, cx, cy, sa, 105, 18, 15, ak);
	ek_ibre(l, cx, cy, dk, 158, 22, 9, ak);
	ek_ibre(l, cx, cy, tm.tm_sec * M_PI / 30, 172, 36, 3, acc);
	rg_dortgen(l, cx - 11, cy - 11, cx + 11, cy + 11, 11, acc, LV_OPA_COVER);
	rg_dortgen(l, cx - 4, cy - 4, cx + 4, cy + 4, 4, lv_color_black(), LV_OPA_COVER);
}

/* dakikada bir yer degistir (saat koruyucusundaki ss_kay_cb ve ss_x0..ss_y1 ile ayni animasyon) */
static void ek_kaydir(int32_t xa, int32_t ya)
{
	time_t now = time(NULL);
	struct tm tm;
	localtime_r(&now, &tm);
	int k = tm.tm_min % 8;
	if (k == ss_last_k || !ek_kutu)
		return;
	int ilk = ss_last_k < 0;
	ss_last_k = k;
	int32_t x = (k % 4) * (2 * xa / 3) - xa, y = (k / 4 ? 1 : -1) * ya;
	if (ilk || lv_anim_get(ek_kutu, ss_kay_cb)) {
		ss_x1 = x;
		ss_y1 = y;
		lv_obj_align(ek_kutu, LV_ALIGN_CENTER, x, y);
		return;
	}
	ss_x0 = ss_x1;
	ss_y0 = ss_y1;
	ss_x1 = x;
	ss_y1 = y;
	lv_anim_t an;
	lv_anim_init(&an);
	lv_anim_set_var(&an, ek_kutu);
	lv_anim_set_exec_cb(&an, ss_kay_cb);
	lv_anim_set_values(&an, 0, 1024);
	lv_anim_set_duration(&an, 1200);
	lv_anim_set_path_cb(&an, lv_anim_path_ease_in_out);
	lv_anim_start(&an);
}

static void ek_metin(lv_obj_t *o, const char *t)
{
	if (o && strcmp(lv_label_get_text(o), t))
		lv_label_set_text(o, t);
}

static void ek_saniye(void)
{
	time_t now = time(NULL);
	struct tm tm;
	char b[256];
	localtime_r(&now, &tm);
	int tarih_var = tm.tm_year > 120;
	snprintf(b, sizeof b, "%02d:%02d", tm.tm_hour, tm.tm_min);
	ek_metin(ek_l1, b);
	if (tarih_var)
		snprintf(b, sizeof b, "%d %s %s", tm.tm_mday, aylar[tm.tm_mon], gunler[tm.tm_wday]);
	else
		b[0] = 0;
	ek_metin(ek_l2, b);
	if (ek_tur == 3) {
		ek_metin(ek_l3, ss_hava);
		lv_obj_invalidate(ek_kadran);
		ek_kaydir(270, 40);
	} else if (ek_tur == 5) {
		ek_metin(ek_l3, ss_hava[0] ? ss_hava : "Hava bilgisi bekleniyor");
		char fx[200] = "";
		const char *f = l_fx ? lv_label_get_text(l_fx) : "";
		for (size_t i = 0, k = 0; f[i] && k + 12 < sizeof fx; i++) {
			if (f[i] == '\n') {
				memcpy(fx + k, "    ·    ", 10);	/* "·" iki bayt: 4+2+4 = 10 */
				k += 10;
			} else
				fx[k++] = f[i];
			fx[k] = 0;
		}
		ek_metin(ek_l4, fx[0] && strcmp(fx, "-") ? fx : "Döviz bilgisi bekleniyor");
		snprintf(b, sizeof b, "%s    ·    Wi-Fi %s    ·    %s", ss_etkinlik[0] ? ss_etkinlik : "Bugün etkinlik yok",
			 strcmp(g_ip, "-") ? g_ip : "bağlı değil", g_pc_ok ? "PC bağlı" : "PC bağlı değil");
		ek_metin(ek_l5, b);
		ek_kaydir(150, 30);
	} else if (ek_tur == 6)
		ek_kaydir(560, 240);
	else if (ek_tur == 7) {	/* 0.44 kelime saati */
		char a1[48], a2[48];
		kelime_saat(tm.tm_hour, tm.tm_min, a1, sizeof a1, a2, sizeof a2);
		ek_metin(ek_l1, a1);
		ek_metin(ek_l2, a2);
		if (tarih_var)
			snprintf(b, sizeof b, "%02d:%02d  ·  %d %s %s", tm.tm_hour, tm.tm_min, tm.tm_mday, aylar[tm.tm_mon],
				 gunler[tm.tm_wday]);
		else
			snprintf(b, sizeof b, "%02d:%02d", tm.tm_hour, tm.tm_min);
		ek_metin(ek_l3, b);
		ek_kaydir(230, 50);
	} else if (ek_tur == 8) {
		dz_guncelle();
		ek_kaydir(120, 60);
	} else if (ek_tur == 9)
		ek_kaydir(560, 230);
	else if (ek_tur == 10) {
		au_guncelle();
		ek_kaydir(300, 110);
	}
	ss_priv_guncelle();
}

static void ek_logo_tmr_cb(lv_timer_t *t)
{
	(void)t;
	if (!ek_kutu)
		return;
	int32_t w = lv_obj_get_width(ek_kutu), h = lv_obj_get_height(ek_kutu), sek = 0;
	ek_x += ek_dx;
	ek_y += ek_dy;
	if (ek_x < 0 || ek_x + w > W) {
		ek_dx = -ek_dx;
		ek_x = ek_x < 0 ? 0 : W - w;
		sek = 1;
	}
	if (ek_y < 0 || ek_y + h > H) {
		ek_dy = -ek_dy;
		ek_y = ek_y < 0 ? 0 : H - h;
		sek = 1;
	}
	lv_obj_set_pos(ek_kutu, ek_x, ek_y);
	if (sek && ek_logo) {	/* her sekiste renk tonu degisir */
		static const uint32_t renk[4] = {0, 0xF97316, 0xFAFAFA, 0x22D3EE};
		ek_renk_i = (ek_renk_i + 1) % 4;
		lv_obj_set_style_image_recolor(ek_logo, ek_renk_i ? lv_color_hex(renk[ek_renk_i]) : C_ACC, 0);
	}
}

static lv_obj_t *ek_etiket(lv_obj_t *par, const lv_font_t *f, uint32_t renk)
{
	lv_obj_t *l = label(par, f, "");
	lv_obj_set_style_text_color(l, lv_color_hex(renk), 0);
	return l;
}

static void ek_goster(int tur)
{
	dm_git(0, 0);
	ek_temizle();
	ss_mod = 10 + tur;	/* cerceve (3) ile karismasin */
	ek_tur = tur;
	ss_obj = lv_obj_create(lv_layer_top());
	lv_obj_remove_style_all(ss_obj);
	lv_obj_set_size(ss_obj, W, H);
	lv_obj_set_style_bg_opa(ss_obj, LV_OPA_COVER, 0);
	lv_obj_set_style_bg_color(ss_obj, lv_color_black(), 0);
	lv_obj_remove_flag(ss_obj, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_add_flag(ss_obj, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_add_event_cb(ss_obj, ss_click_cb, LV_EVENT_PRESSED, NULL);
	ek_kutu = lv_obj_create(ss_obj);
	lv_obj_remove_style_all(ek_kutu);
	lv_obj_set_size(ek_kutu, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
	lv_obj_remove_flag(ek_kutu, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_t *col = ek_kutu;
	if (tur == 3) {	/* analog: [kadran] [saat / tarih / hava] */
		lv_obj_set_flex_flow(ek_kutu, LV_FLEX_FLOW_ROW);
		lv_obj_set_flex_align(ek_kutu, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
		lv_obj_set_style_pad_column(ek_kutu, 56, 0);
		ek_kadran = lv_obj_create(ek_kutu);
		lv_obj_remove_style_all(ek_kadran);
		lv_obj_set_size(ek_kadran, 2 * EK_R + 12, 2 * EK_R + 12);
		lv_obj_remove_flag(ek_kadran, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
		lv_obj_add_event_cb(ek_kadran, ek_kadran_cb, LV_EVENT_DRAW_MAIN, NULL);
		col = kutu(ek_kutu, LV_FLEX_FLOW_COLUMN, 6);
		lv_obj_set_width(col, 520);
		ek_l1 = ek_etiket(col, f_xl, 0xD4D4D8);
		ek_l2 = ek_etiket(col, f_l, 0);
		lv_obj_set_style_text_color(ek_l2, C_ACC, 0);
		ek_l3 = ek_etiket(col, f_m, 0x71717A);
	} else if (tur == 4) {	/* kayan logo */
		lv_obj_set_flex_flow(ek_kutu, LV_FLEX_FLOW_COLUMN);
		lv_obj_set_flex_align(ek_kutu, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
		lv_obj_set_style_pad_row(ek_kutu, 8, 0);
		ek_logo = lv_image_create(ek_kutu);
		lv_image_set_src(ek_logo, &logo_tam);
		lv_image_set_scale(ek_logo, 352);	/* 256 = %100 */
		lv_obj_set_size(ek_logo, logo_tam.header.w * 352 / 256, logo_tam.header.h * 352 / 256);
		lv_image_set_inner_align(ek_logo, LV_IMAGE_ALIGN_CENTER);
		lv_obj_set_style_image_recolor(ek_logo, C_ACC, 0);
		lv_obj_set_style_image_recolor_opa(ek_logo, LV_OPA_40, 0);
		ek_l1 = ek_etiket(ek_kutu, f_l, 0x71717A);
		ek_renk_i = 0;
	} else if (tur == 5) {	/* bilgi panosu */
		lv_obj_set_flex_flow(ek_kutu, LV_FLEX_FLOW_COLUMN);
		lv_obj_set_style_pad_row(ek_kutu, 8, 0);
		lv_obj_set_width(ek_kutu, 1180);
		ek_l1 = ek_etiket(ek_kutu, f_clock, 0xD4D4D8);
		ek_l2 = ek_etiket(ek_kutu, f_l, 0);
		lv_obj_set_style_text_color(ek_l2, C_ACC, 0);
		ek_l3 = ek_etiket(ek_kutu, f_m, 0xA1A1AA);
		ek_l4 = ek_etiket(ek_kutu, f_m, 0x8A8A93);
		ek_l5 = ek_etiket(ek_kutu, f_m, 0x6B6B73);
		lv_obj_t *ls[3] = {ek_l3, ek_l4, ek_l5};
		for (int i = 0; i < 3; i++) {
			lv_obj_set_width(ls[i], LV_PCT(100));
			lv_label_set_long_mode(ls[i], LV_LABEL_LONG_DOT);
			lv_obj_set_height(ls[i], lv_font_get_line_height(f_m));
		}
		lv_obj_set_style_margin_top(ek_l3, 10, 0);	/* pad degil: sabit yukseklikte metni asagi iterdi */
	} else if (tur == 6) {	/* karartma */
		lv_obj_set_flex_flow(ek_kutu, LV_FLEX_FLOW_COLUMN);
		ek_l1 = ek_etiket(ek_kutu, f_m, 0x3F3F46);
	} else if (tur == 7) {	/* 0.44 kelime saati: ek_l1/ek_l2 sozcukler, ek_l3 dijital + tarih */
		lv_obj_set_flex_flow(ek_kutu, LV_FLEX_FLOW_COLUMN);
		lv_obj_set_style_pad_row(ek_kutu, 2, 0);
		ek_l1 = ek_etiket(ek_kutu, f_xl, 0xE7E5E4);
		ek_l2 = ek_etiket(ek_kutu, f_xl, 0);
		lv_obj_set_style_text_color(ek_l2, C_ACC, 0);
		ek_l3 = ek_etiket(ek_kutu, f_m, 0x71717A);
		lv_obj_set_style_margin_top(ek_l3, 14, 0);
	} else if (tur == 8) {	/* 0.44 dunya saatleri: dort sutun */
		lv_obj_set_flex_flow(ek_kutu, LV_FLEX_FLOW_ROW);
		lv_obj_set_style_pad_column(ek_kutu, 64, 0);
		for (int i = 0; i < 4; i++) {
			lv_obj_t *k = kutu(ek_kutu, LV_FLEX_FLOW_COLUMN, 4);
			lv_obj_set_width(k, LV_SIZE_CONTENT);
			dz_l[i][0] = ek_etiket(k, f_m, 0x8A8A93);
			dz_l[i][1] = ek_etiket(k, f_xl, i ? 0xE7E5E4 : 0);
			if (!i)
				lv_obj_set_style_text_color(dz_l[i][1], C_ACC, 0);
			dz_l[i][2] = ek_etiket(k, f_s, 0x5F5F66);
		}
		col = lv_obj_get_child(ek_kutu, 0);
	} else if (tur == 9) {	/* 0.44 yildiz alani: yildizlar tam ekran nesnede, saat ek_kutu'da */
		ek_yildiz = lv_obj_create(ss_obj);
		lv_obj_remove_style_all(ek_yildiz);
		lv_obj_set_size(ek_yildiz, W, H);
		lv_obj_remove_flag(ek_yildiz, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
		lv_obj_add_event_cb(ek_yildiz, yz_ciz_cb, LV_EVENT_DRAW_MAIN, NULL);
		lv_obj_move_background(ek_yildiz);
		for (int i = 0; i < YZ_N; i++)
			yz_yeni(i, 0.08f + yz_rast() * 0.92f);
		lv_obj_set_flex_flow(ek_kutu, LV_FLEX_FLOW_COLUMN);
		ek_l1 = ek_etiket(ek_kutu, f_m, 0x52525B);
	} else {	/* 0.44 aurora (10): ss_obj dikey gradyan, saat + tarih */
		lv_obj_set_style_bg_grad_dir(ss_obj, LV_GRAD_DIR_VER, 0);
		lv_obj_set_flex_flow(ek_kutu, LV_FLEX_FLOW_COLUMN);
		lv_obj_set_style_pad_row(ek_kutu, 4, 0);
		ek_l1 = ek_etiket(ek_kutu, f_xl, 0xD4D4D8);
		ek_l2 = ek_etiket(ek_kutu, f_m, 0x9A9AA2);
	}
	ss_priv = hap(col);
	lv_obj_set_style_margin_top(ss_priv.kap, 10, 0);
	lv_obj_add_flag(ss_priv.kap, LV_OBJ_FLAG_HIDDEN);
	ss_alti_gizle(1);
	ss_shown = tur == 6 ? 3 : 1;
	ss_last_k = -1;
	ek_saniye();
	if (tur == 9) {	/* 0.44 */
		const char *k = getenv("PANEL_YZ_KIRPMA"), *tt = getenv("PANEL_YZ_TEST");	/* 0.62 sinama kancalari */
		yz_kirpma = !(k && k[0] == '0');
		yz_dondur = tt && !strcmp(tt, "dondur");
		if (yz_dondur)
			lv_obj_add_flag(ek_kutu, LV_OBJ_FLAG_HIDDEN);
		yz_o_t0 = tick_ms();
		yz_o_kare = yz_o_azami = 0;
		yz_o_us = 0;
		yz_o_g2d0 = pg_say;
		yz_o_g2dus0 = pg_us;
		ek_tmr = lv_timer_create(yz_tmr_cb, 110, NULL);
	}
	if (tur == 4) {
		lv_obj_update_layout(ek_kutu);
		ek_x = (W - lv_obj_get_width(ek_kutu)) / 3;
		ek_y = (H - lv_obj_get_height(ek_kutu)) / 4;
		lv_obj_set_pos(ek_kutu, ek_x, ek_y);
		ek_tmr = lv_timer_create(ek_logo_tmr_cb, 80, NULL);
	}
	br_cur = -1;
	br_apply();
	printf("koruyucu: %s\n", ss_tur_kod[tur]);
}

/* ------------------------------------------------------------ 0.37 dijital cerceve */
/* Ekran koruyucu yerine SD karttaki fotograflar (cerceve_v1.h). ss_mod 3, ss_shown 2 (parlaklik kisilmaz). */
#define CF_BAYRAK "/data/panel-cerceve"
#define CF_DIZIN "/mnt/sd/fotograf"
#define CF_DIZIN2 "/mnt/sd/kamera/fotolar"
#define CF_SURE_MS 20000u
#define CF_AZAMI 400
static int cf_acik = -1;	/* -1: bayrak henuz okunmadi */
static char *cf_ad[CF_AZAMI];
static int cf_n, cf_sira, cf_hata_ust;
static char cf_kok[64];
static uint16_t *cf_tam[2];
static lv_image_dsc_t cf_dsc[2];
static int cf_on;	/* gosterilen tampon */
static lv_obj_t *cf_img, *cf_saat, *cf_alt;
static lv_timer_t *cf_tmr;
static pthread_t cf_th;
static volatile int cf_is, cf_iptal;	/* cf_is: 0 bos, 1 cozuyor, 2 bitti (join bekliyor) */
static int cf_sonuc, cf_ow, cf_oh, cf_hazir, cf_hw, cf_hh, cf_gosterilen;
static char cf_yol[320], cf_hata[128];
static uint32_t cf_t_gos, cf_t_coz;
static int cf_son_dk = -1;

static int cf_acik_mi(void)
{
	if (cf_acik < 0) {
		struct stat st;
		cf_acik = !stat(CF_BAYRAK, &st);
	}
	return cf_acik;
}

static void cf_liste_bosalt(void)
{
	for (int i = 0; i < cf_n; i++)
		free(cf_ad[i]);
	cf_n = cf_sira = 0;
}

static int cf_liste_dizin(const char *kok)
{
	DIR *dd = opendir(kok);
	struct dirent *e;
	if (!dd)
		return 0;
	while ((e = readdir(dd)) && cf_n < CF_AZAMI)
		if (cf_jpg_mi(e->d_name))
			cf_ad[cf_n++] = strdup(e->d_name);
	closedir(dd);
	if (cf_n)
		snprintf(cf_kok, sizeof cf_kok, "%s", kok);
	return cf_n;
}

static int cf_listele(void)
{
	cf_liste_bosalt();
	if (!cf_liste_dizin(CF_DIZIN))
		cf_liste_dizin(CF_DIZIN2);
	for (int i = cf_n - 1; i > 0; i--) {	/* karistir */
		int j = rand() % (i + 1);
		char *t = cf_ad[i];
		cf_ad[i] = cf_ad[j];
		cf_ad[j] = t;
	}
	return cf_n;
}

static void *cf_is_parcacigi(void *u)
{
	(void)u;
	setpriority(PRIO_PROCESS, (id_t)syscall(SYS_gettid), 15);	/* arayuz ve Hey Kole once */
	int w = 0, h = 0;
	cf_sonuc = cf_coz(cf_yol, cf_tam[1 - cf_on], W, H, &w, &h, &cf_iptal, cf_hata, sizeof cf_hata);
	cf_ow = w;
	cf_oh = h;
	__atomic_store_n(&cf_is, 2, __ATOMIC_RELEASE);
	return NULL;
}

static void cf_coz_baslat(void)
{
	if (!cf_n || cf_is)
		return;
	if (cf_sira >= cf_n && !cf_listele())
		return;
	snprintf(cf_yol, sizeof cf_yol, "%s/%s", cf_kok, cf_ad[cf_sira++ % cf_n]);
	cf_iptal = 0;
	cf_is = 1;
	cf_t_coz = tick_ms();
	if (pthread_create(&cf_th, NULL, cf_is_parcacigi, NULL)) {
		cf_is = 0;
		cf_hata_ust++;
	}
}

static void cf_saniye(void)
{
	time_t now = time(NULL);
	struct tm tm;
	if (!cf_saat)
		return;
	localtime_r(&now, &tm);
	if (tm.tm_min != cf_son_dk) {
		cf_son_dk = tm.tm_min;
		if (tm.tm_year > 120)
			lv_label_set_text_fmt(cf_saat, "%02d:%02d  ·  %d %s", tm.tm_hour, tm.tm_min, tm.tm_mday, aylar[tm.tm_mon]);
		else
			lv_label_set_text_fmt(cf_saat, "%02d:%02d", tm.tm_hour, tm.tm_min);
		/* iz kalmasin: 15 dakikada bir kose degistir */
		static const lv_align_t k[4] = {LV_ALIGN_BOTTOM_RIGHT, LV_ALIGN_BOTTOM_LEFT, LV_ALIGN_TOP_RIGHT, LV_ALIGN_TOP_LEFT};
		lv_obj_align(cf_alt, k[tm.tm_min / 15 % 4], (tm.tm_min / 15 % 2) ? 24 : -24, (tm.tm_min / 15 >= 2) ? 20 : -20);
	}
	ss_priv_guncelle();
}

static void cf_goster_hazir(void)
{
	cf_on = 1 - cf_on;
	lv_image_dsc_t *d = &cf_dsc[cf_on];
	memset(d, 0, sizeof *d);
	d->header.magic = LV_IMAGE_HEADER_MAGIC;
	d->header.cf = LV_COLOR_FORMAT_RGB565;
	d->header.w = (uint32_t)cf_hw;
	d->header.h = (uint32_t)cf_hh;
	d->header.stride = (uint32_t)cf_hw * 2;
	d->data_size = (uint32_t)cf_hw * cf_hh * 2;
	d->data = (const uint8_t *)cf_tam[cf_on];
	lv_image_cache_drop(d);
	lv_image_set_src(cf_img, d);
	lv_obj_set_size(cf_img, cf_hw, cf_hh);
	lv_obj_center(cf_img);
	lv_obj_invalidate(ss_obj);
	cf_hazir = 0;
	cf_gosterilen++;
	cf_t_gos = tick_ms();
	lv_obj_move_foreground(cf_alt);
}

static void cf_tmr_cb(lv_timer_t *t)
{
	(void)t;
	if (!cf_img)
		return;
	if (cf_is == 2 && __atomic_load_n(&cf_is, __ATOMIC_ACQUIRE) == 2) {
		pthread_join(cf_th, NULL);
		cf_is = 0;
		if (!cf_sonuc) {
			cf_hazir = 1;
			cf_hw = cf_ow;
			cf_hh = cf_oh;
			cf_hata_ust = 0;
			printf("cerceve: %s %s (%u ms)\n", cf_yol, cf_hata, (unsigned)(tick_ms() - cf_t_coz));
		} else {
			cf_hata_ust++;
			printf("cerceve: ATLANDI %s: %s\n", cf_yol, cf_hata);
		}
	}
	if (cf_hazir && (!cf_gosterilen || tick_ms() - cf_t_gos >= CF_SURE_MS)) {
		cf_goster_hazir();
		lv_label_set_text(cf_saat, "");
		cf_son_dk = -1;
		cf_saniye();
	}
	if (!cf_is && !cf_hazir) {
		if (cf_n == 1 && cf_gosterilen)	/* tek fotograf: yeniden cozme */
			return;
		if (cf_hata_ust > cf_n + 2) {	/* hicbiri cozulemiyor */
			if (!cf_gosterilen && cf_saat) {
				lv_label_set_text(cf_saat, "Fotoğraflar açılamadı (progresif JPEG desteklenmiyor)");
				lv_obj_align(cf_alt, LV_ALIGN_CENTER, 0, 0);
			}
			return;
		}
		cf_coz_baslat();
	}
}

/* Nesneler (ss_obj altinda) silinmis olabilir: yalniz is parcacigi, zamanlayici, tampon ve isaretciler. */
static void cf_temizle(void)
{
	if (cf_tmr)
		lv_timer_delete(cf_tmr);
	cf_tmr = NULL;
	if (cf_is) {
		cf_iptal = 1;
		pthread_join(cf_th, NULL);
		cf_is = 0;
	}
	for (int i = 0; i < 2; i++) {
		if (cf_tam[i])
			lv_image_cache_drop(&cf_dsc[i]);
		free(cf_tam[i]);
		cf_tam[i] = NULL;
	}
	cf_img = cf_saat = cf_alt = NULL;
	cf_hazir = cf_gosterilen = 0;
	cf_liste_bosalt();
}

/* 1: cerceve acildi; 0: fotograf yok / bellek yok (cagiran normal koruyucuyu acar) */
static int cf_goster(void)
{
	if (ss_obj)
		return 1;
	if (!cf_listele()) {
		printf("cerceve: %s ve %s icinde JPEG yok\n", CF_DIZIN, CF_DIZIN2);
		return 0;
	}
	for (int i = 0; i < 2; i++)
		cf_tam[i] = malloc((size_t)W * H * 2);
	if (!cf_tam[0] || !cf_tam[1]) {
		printf("cerceve: tampon icin bellek yok\n");
		cf_temizle();
		return 0;
	}
	dm_git(0, 0);
	ss_mod = 3;
	ss_obj = lv_obj_create(lv_layer_top());
	lv_obj_remove_style_all(ss_obj);
	lv_obj_set_size(ss_obj, W, H);
	lv_obj_set_style_bg_opa(ss_obj, LV_OPA_COVER, 0);
	lv_obj_set_style_bg_color(ss_obj, lv_color_black(), 0);
	lv_obj_remove_flag(ss_obj, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_add_flag(ss_obj, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_add_event_cb(ss_obj, ss_click_cb, LV_EVENT_PRESSED, NULL);
	cf_img = lv_image_create(ss_obj);
	lv_obj_remove_flag(cf_img, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_set_size(cf_img, 1, 1);
	cf_alt = lv_obj_create(ss_obj);
	lv_obj_remove_style_all(cf_alt);
	lv_obj_set_size(cf_alt, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
	lv_obj_set_flex_flow(cf_alt, LV_FLEX_FLOW_ROW);
	lv_obj_set_flex_align(cf_alt, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	lv_obj_set_style_pad_column(cf_alt, 12, 0);
	lv_obj_set_style_pad_hor(cf_alt, 18, 0);
	lv_obj_set_style_pad_ver(cf_alt, 8, 0);
	lv_obj_set_style_radius(cf_alt, 22, 0);
	lv_obj_set_style_bg_color(cf_alt, lv_color_black(), 0);
	lv_obj_set_style_bg_opa(cf_alt, LV_OPA_50, 0);
	lv_obj_remove_flag(cf_alt, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
	cf_saat = label(cf_alt, f_m, "");
	lv_obj_set_style_text_color(cf_saat, lv_color_hex(0xE4E4E7), 0);
	ss_priv = hap(cf_alt);
	lv_obj_add_flag(ss_priv.kap, LV_OBJ_FLAG_HIDDEN);
	lv_obj_align(cf_alt, LV_ALIGN_BOTTOM_RIGHT, -24, -20);
	ss_alti_gizle(1);
	cf_on = 0;
	cf_is = cf_iptal = cf_hazir = cf_gosterilen = cf_hata_ust = 0;
	cf_son_dk = -1;
	cf_tmr = lv_timer_create(cf_tmr_cb, 200, NULL);
	cf_coz_baslat();
	ss_shown = 2;
	ss_last_k = -1;
	br_cur = -1;
	br_apply();
	printf("koruyucu: cerceve (%d fotograf, %s)\n", cf_n, cf_kok);
	return 1;
}

static void cf_ayarla(int ac)
{
	cf_acik = ac ? 1 : 0;
	if (ac) {
		int fd = open(CF_BAYRAK, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
		if (fd >= 0)
			close(fd);
	} else {
		unlink(CF_BAYRAK);
	}
	sync();
	printf("cerceve: %s\n", ac ? "acik" : "kapali");
}

static void ss_show(void)
{
	if (!gece_aktif && cf_acik_mi() && cf_goster())	/* 0.37; 0.41 gece cerceve yok */
		return;
	ss_show_tur(ss_tur);	/* 0.48: gece de secili koruyucu (kullanıcı: "göz gelmiyor"); 0.41 gece Karartma idi */
}

/* 0.41 gece kipi: 23:00-07:00 (saat gecerliyse). PANEL_GECE_ZORLA=0|1 yalniz PC provasi. Saniyede bir. */
static int gece_simdi(void)
{
	const char *z = getenv("PANEL_GECE_ZORLA");
	if (z && (z[0] == '0' || z[0] == '1'))
		return z[0] - '0';
	time_t now = time(NULL);
	struct tm tm;
	localtime_r(&now, &tm);
	return tm.tm_year > 120 && (tm.tm_hour >= 23 || tm.tm_hour < 7);
}

static void gece_tick(void)
{
	int yeni = gece_ayar && gece_simdi();
	if (yeni == gece_aktif)
		return;
	gece_aktif = yeni;
	printf("gece kipi: %s\n", yeni ? "basladi (rahatsiz etme, parlaklik <= %30, koruyucu secili tur)" : "bitti");
	br_cur = -1;
	br_apply();
}

static void ss_tick(void)
{
	if (ss_obj) {
		if (rg_obj)
			rg_saniye();
		else if (cf_img)	/* 0.37 */
			cf_saniye();
		else if (ek_kutu)	/* 0.40 */
			ek_saniye();
		else
			ss_update();
	} else if (ss_on && lv_display_get_inactive_time(disp) > SS_MS) {
		printf("koruyucu: otomatik (%d dk etkinsiz, %u pasif ui iletisi)\n", ss_dk, (unsigned)ui_pasif_n);	/* 0.72 */
		ss_show();
	}
}

static lv_obj_t *ay_sw_cf;	/* 0.37 */
static lv_obj_t *ay_sw_gece;	/* 0.41 */
static void gece_ayarla(int on)
{
	char yol[160];
	gece_ayar = !!on;
	veri_yol(yol, sizeof yol, "panel-gece-kapali");
	if (gece_ayar)
		unlink(yol);
	else {
		int fd = open(yol, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
		if (fd >= 0)
			close(fd);
	}
	printf("gece kipi ayari: %s\n", gece_ayar ? "acik" : "kapali");
	gece_tick();
}

static void gece_sw_cb(lv_event_t *e)
{
	(void)e;
	gece_ayarla(lv_obj_has_state(ay_sw_gece, LV_STATE_CHECKED));
}

static void cf_sw_cb(lv_event_t *e)
{
	(void)e;
	cf_ayarla(lv_obj_has_state(ay_sw_cf, LV_STATE_CHECKED));
}

static void ss_sw_cb(lv_event_t *e)
{
	ss_on = lv_obj_has_state(sw_ss, LV_STATE_CHECKED);
	br_save();
}

/* --------------------------------------------------------------- zamanlayici */
static lv_obj_t *tm_label, *tm_mode_l, *tm_arc, *tm_start_l;
static int tm_total = 25 * 60, tm_left = 25 * 60, tm_run;
static const char *tm_mode = "Odak";
static uint32_t tm_last_ms;

static void tm_draw(void)
{
	if (!tm_label)
		return;
	lv_label_set_text_fmt(tm_label, "%02d:%02d", tm_left / 60, tm_left % 60);
	lv_label_set_text_fmt(tm_mode_l, "%s · %d dk", tm_mode, tm_total / 60);
	lv_arc_set_value(tm_arc, tm_total ? (tm_total - tm_left) * 1000 / tm_total : 0);
	lv_label_set_text(tm_start_l, tm_run ? LV_SYMBOL_PAUSE "  Duraklat" : LV_SYMBOL_PLAY "  Başlat");
}

static void tm_set(const char *mode, int min)
{
	tm_mode = mode;
	tm_total = tm_left = min * 60;
	tm_run = 0;
	tm_draw();
}

static void tm_btn_cb(lv_event_t *e)
{
	int v = (int)(uintptr_t)lv_event_get_user_data(e);

	if (v == 25)
		tm_set("Odak", 25);
	else if (v == 5)
		tm_set("Mola", 5);
	else if (v == 15)
		tm_set("Uzun mola", 15);
	else if (v == 1) {	/* baslat/duraklat */
		if (!tm_left)
			tm_left = tm_total;
		tm_run = !tm_run;
		tm_last_ms = tick_ms();
	} else if (v == 0) {	/* sifirla */
		tm_left = tm_total;
		tm_run = 0;
	}
	tm_draw();
}

static void tm_tick(void)
{
	if (!tm_run)
		return;
	uint32_t now = tick_ms();
	while (now - tm_last_ms >= 1000 && tm_left > 0) {
		tm_left--;
		tm_last_ms += 1000;
	}
	if (tm_left == 0) {
		tm_run = 0;
		char ev[128];
		snprintf(ev, sizeof ev, "{\"t\":\"timer\",\"state\":\"done\",\"mode\":\"%s\"}", tm_mode);
		send_event(ev);
		if (ss_obj)
			ss_hide();
		lv_display_trigger_activity(disp);
		show_toast(LV_SYMBOL_BELL "  Süre doldu!");
		if (!pc_ajan_var() && !dnd && !ses_sessiz) {	/* 0.52: PC yoksa zil cihaz hoparlorunden */
			int r = sk_zil_baslat(ses_duzey, ses_sessiz);
			printf("zamanlayici zili: %s\n", r == 0 ? "basladi" : r == -1 ? "baska calma suruyor" : "calinamadi");
		} else {
			printf("zamanlayici zili: atlandi (%s)\n", pc_ajan_var() ? "PC ajani calar" : "sessiz/rahatsiz etme");
		}
	}
	tm_draw();
}

static lv_obj_t *tm_button(lv_obj_t *par, const char *txt, int v, int w)
{
	lv_obj_t *b = lv_button_create(par);
	lv_obj_set_size(b, w, 92);
	lv_obj_set_style_radius(b, 16, 0);
	lv_obj_set_style_bg_color(b, lv_color_hex(TH.btn), 0);
	lv_obj_set_style_bg_color(b, C_ACC, LV_STATE_PRESSED);
	lv_obj_set_style_text_color(b, lv_color_hex(TH.text), 0);	/* 0.22: Beyaz temada okunurluk */
	lv_obj_set_style_text_color(b, lv_color_white(), LV_STATE_PRESSED);
	lv_obj_set_style_border_width(b, 1, 0);
	lv_obj_set_style_border_color(b, lv_color_hex(TH.sep), 0);
	lv_obj_set_style_shadow_width(b, 0, 0);
	lv_obj_t *l = label(b, f_mb, txt);
	lv_obj_center(l);
	lv_obj_add_event_cb(b, tm_btn_cb, LV_EVENT_CLICKED, (void *)(uintptr_t)v);
	return l;
}

static void tm_build(lv_obj_t *t)
{
	lv_obj_set_flex_flow(t, LV_FLEX_FLOW_ROW);
	lv_obj_set_style_pad_column(t, 14, 0);
	lv_obj_remove_flag(t, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_t *c = card(t, NULL);
	lv_obj_set_flex_grow(c, 5);
	lv_obj_set_flex_align(c, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	tm_arc = lv_arc_create(c);
	lv_obj_set_size(tm_arc, 400, 400);
	lv_arc_set_rotation(tm_arc, 270);
	lv_arc_set_bg_angles(tm_arc, 0, 360);
	lv_arc_set_range(tm_arc, 0, 1000);
	lv_obj_remove_style(tm_arc, NULL, LV_PART_KNOB);
	lv_obj_remove_flag(tm_arc, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_set_style_arc_width(tm_arc, 22, LV_PART_MAIN);
	lv_obj_set_style_arc_width(tm_arc, 22, LV_PART_INDICATOR);
	lv_obj_set_style_arc_color(tm_arc, C_ACC, LV_PART_INDICATOR);
	lv_obj_set_style_arc_color(tm_arc, lv_color_hex(TH.sep), LV_PART_MAIN);	/* 0.22: tema rengi */
	lv_obj_set_style_arc_rounded(tm_arc, true, LV_PART_INDICATOR);
	tm_label = label(tm_arc, f_xl, "");
	lv_obj_set_style_text_font(tm_label, f_clock, 0);
	lv_obj_align(tm_label, LV_ALIGN_CENTER, 0, -14);
	lv_obj_set_style_transform_scale(tm_label, 150, 0);	/* 150/256: halkaya sigsin */
	lv_obj_set_style_transform_pivot_x(tm_label, LV_PCT(50), 0);
	lv_obj_set_style_transform_pivot_y(tm_label, LV_PCT(50), 0);
	tm_mode_l = label(tm_arc, f_m, "");
	lv_obj_set_style_text_color(tm_mode_l, C_DIM, 0);
	lv_obj_align(tm_mode_l, LV_ALIGN_CENTER, 0, 88);

	lv_obj_t *r = card_i(t, LV_SYMBOL_LOOP, "ZAMANLAYICI");
	lv_obj_set_flex_grow(r, 5);
	lv_obj_t *row = lv_obj_create(r);
	lv_obj_remove_style_all(row);
	lv_obj_set_size(row, LV_PCT(100), LV_SIZE_CONTENT);
	lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
	lv_obj_set_style_pad_column(row, 12, 0);
	tm_button(row, "Odak 25", 25, 220);
	tm_button(row, "Mola 5", 5, 200);
	tm_button(row, "Uzun 15", 15, 200);
	lv_obj_t *row2 = lv_obj_create(r);
	lv_obj_remove_style_all(row2);
	lv_obj_set_size(row2, LV_PCT(100), LV_SIZE_CONTENT);
	lv_obj_set_flex_flow(row2, LV_FLEX_FLOW_ROW);
	lv_obj_set_style_pad_column(row2, 12, 0);
	lv_obj_set_style_pad_top(row2, 12, 0);
	tm_start_l = tm_button(row2, "", 1, 340);
	tm_button(row2, LV_SYMBOL_REFRESH "  Sıfırla", 0, 290);
	lv_obj_t *hint = label(r, f_s, "\nSüre dolunca panelde uyarı çıkar, PC ajanı bilgisayardan sesli bildirim verir.");
	lv_label_set_long_mode(hint, LV_LABEL_LONG_WRAP);
	lv_obj_set_width(hint, LV_PCT(100));
	lv_obj_set_style_text_color(hint, C_DIM, 0);
	tm_draw();
}

/* ------------------------------------------------------------ Wi-Fi kurulumu */
#define WF_MAX 24
typedef struct { char ssid[64]; int sig; int sec; } wf_ag_t;
static wf_ag_t wf_list[WF_MAX];
static int wf_n, wf_state;	/* 0 bos, 1 taraniyor, 2 tarama bitti, 3 baglaniyor, 4 baglandi, 5 hata */
static char wf_msg[160];
static char wf_sel[64], wf_pw[96];
static lv_obj_t *wf_listobj, *wf_status, *wf_ta, *wf_sel_l, *wf_kb;
static int wf_drawn_state = -1;

static void sh_quote(char *o, size_t n, const char *in)
{
	size_t k = 0;
	if (n < 3)
		return;
	o[k++] = '\'';
	for (; *in && k + 5 < n; in++) {
		if (*in == '\'') {
			memcpy(o + k, "'\\''", 4);
			k += 4;
		} else
			o[k++] = *in;
	}
	o[k++] = '\'';
	o[k] = 0;
}

static void *wf_scan_thread(void *a)
{
	(void)a;
	if (system("lsmod | grep -q '^ssv6x5x' || insmod /lib/wifi/ssv6006.ko; i=0; "
	       "while [ ! -e /sys/class/net/wlan0 ] && [ $i -lt 8 ]; do sleep 1; i=$((i+1)); done; ip link set wlan0 up")) {
	}
	wf_ag_t tmp[WF_MAX];
	int n = 0, cur = -1;
	FILE *f = popen("iw dev wlan0 scan 2>/dev/null", "r");
	char line[256];
	while (f && fgets(line, sizeof line, f)) {
		char *q;
		if (!strncmp(line, "BSS ", 4)) {
			cur = n < WF_MAX ? n : -1;
			if (cur >= 0) {
				memset(&tmp[cur], 0, sizeof tmp[cur]);
				tmp[cur].sig = -100;
				n++;
			}
		} else if (cur >= 0 && (q = strstr(line, "signal:"))) {
			tmp[cur].sig = atoi(q + 7);
		} else if (cur >= 0 && (q = strstr(line, "SSID: "))) {
			q += 6;
			q[strcspn(q, "\r\n")] = 0;
			snprintf(tmp[cur].ssid, sizeof tmp[cur].ssid, "%s", q);
		} else if (cur >= 0 && (strstr(line, "RSN:") || strstr(line, "WPA:"))) {
			tmp[cur].sec = 1;
		}
	}
	if (f)
		pclose(f);
	/* ayni SSID'leri birlestir (en guclu), gizlileri at, sinyale gore sirala */
	wf_ag_t out[WF_MAX];
	int m = 0;
	for (int i = 0; i < n; i++) {
		if (!tmp[i].ssid[0])
			continue;
		int j;
		for (j = 0; j < m; j++)
			if (!strcmp(out[j].ssid, tmp[i].ssid))
				break;
		if (j == m)
			out[m++] = tmp[i];
		else if (tmp[i].sig > out[j].sig)
			out[j].sig = tmp[i].sig;
	}
	for (int i = 0; i < m; i++)
		for (int j = i + 1; j < m; j++)
			if (out[j].sig > out[i].sig) {
				wf_ag_t t = out[i];
				out[i] = out[j];
				out[j] = t;
			}
	pthread_mutex_lock(&mu);
	memcpy(wf_list, out, sizeof out);
	wf_n = m;
	wf_state = 2;
	snprintf(wf_msg, sizeof wf_msg, m ? "%d ağ bulundu. Bağlanmak için birine dokun." : "Ağ bulunamadı. Yenile'ye dokun.", m);
	pthread_mutex_unlock(&mu);
	return NULL;
}

static void *wf_connect_thread(void *a)
{
	char qs[200], qp[300], cmd[600];
	(void)a;
	sh_quote(qs, sizeof qs, wf_sel);
	sh_quote(qp, sizeof qp, wf_pw);
	snprintf(cmd, sizeof cmd, "wifi-kaydet %s %s > /tmp/wifi-kaydet.log 2>&1", qs, qp);
	memset(wf_pw, 0, sizeof wf_pw);
	int rc = system(cmd), st = rc != -1 && WIFEXITED(rc) ? WEXITSTATUS(rc) : -1;	/* 0.77: wifi_kaydet_sd_v1 */
	int geri = st == 4, kayitsiz = st == 1 || st == 2;	/* 4 eski ayar geri; 1/2 kaydedilmedi (eski ag bagli kalir) */
	memset(cmd, 0, sizeof cmd);
	memset(qp, 0, sizeof qp);
	int ok = 0;
	for (int i = 0; i < 25 && !ok && !geri && !kayitsiz; i++) {
		sleep(1);
		ok = system("ip -4 addr show wlan0 | grep -q inet") == 0;
	}
	pthread_mutex_lock(&mu);
	wf_state = ok ? 4 : 5;
	snprintf(wf_msg, sizeof wf_msg, kayitsiz ? "Kaydedilmedi: %s (geçersiz ad/şifre ya da /data yazılamadı)." : ok ? "Bağlandı: %s (kaydedildi)"
		 : geri ? "Bağlanamadı: %s. Eski Wi-Fi ayarı geri yüklendi." : "Bağlanamadı: %s. Şifreyi kontrol et.", wf_sel);
	pthread_mutex_unlock(&mu);
	return NULL;
}

static void wf_start_scan(void)
{
	pthread_t t;
	pthread_mutex_lock(&mu);
	if (wf_state == 1 || wf_state == 3) {	/* 0.79: tarama/baglanma surerken pencere yeniden acilirsa ikinci is yok (A20) */
		pthread_mutex_unlock(&mu);
		return;
	}
	wf_state = 1;
	snprintf(wf_msg, sizeof wf_msg, "Taranıyor...");
	pthread_mutex_unlock(&mu);
	pthread_create(&t, NULL, wf_scan_thread, NULL);
	pthread_detach(t);
}

static void wf_close_cb(lv_event_t *e)
{
	if (wf_win) {
		lv_obj_delete(wf_win);
		wf_win = NULL;
	}
}

static void wf_rescan_cb(lv_event_t *e)
{
	if (wf_state != 1 && wf_state != 3)
		wf_start_scan();
}

static void wf_pick_cb(lv_event_t *e)
{
	int i = (int)(uintptr_t)lv_event_get_user_data(e);
	pthread_mutex_lock(&mu);
	if (i >= 0 && i < wf_n)
		snprintf(wf_sel, sizeof wf_sel, "%s", wf_list[i].ssid);
	pthread_mutex_unlock(&mu);
	lv_label_set_text_fmt(wf_sel_l, LV_SYMBOL_WIFI "  %s", wf_sel);
	lv_textarea_set_text(wf_ta, "");
	lv_obj_remove_flag(wf_kb, LV_OBJ_FLAG_HIDDEN);
}

static void wf_connect_cb(lv_event_t *e)
{
	pthread_t t;
	if (!wf_sel[0] || wf_state == 3)
		return;
	snprintf(wf_pw, sizeof wf_pw, "%s", lv_textarea_get_text(wf_ta));
	if (strlen(wf_pw) < 8) {
		lv_label_set_text(wf_status, "Şifre en az 8 karakter olmalı (WPA2).");
		return;
	}
	if (strlen(wf_pw) > 63) {	/* 0.77: WPA2 parolasi en cok 63 karakter; uzunu wpa.conf'u bozuyordu */
		lv_label_set_text(wf_status, "Şifre en çok 63 karakter olabilir (WPA2).");
		return;
	}
	pthread_mutex_lock(&mu);
	wf_state = 3;
	snprintf(wf_msg, sizeof wf_msg, "Bağlanıyor: %s ...", wf_sel);
	pthread_mutex_unlock(&mu);
	lv_obj_add_flag(wf_kb, LV_OBJ_FLAG_HIDDEN);
	pthread_create(&t, NULL, wf_connect_thread, NULL);
	pthread_detach(t);
}

static void wf_kb_cb(lv_event_t *e)
{
	lv_event_code_t c = lv_event_get_code(e);
	if (c == LV_EVENT_READY)
		wf_connect_cb(e);
	else if (c == LV_EVENT_CANCEL)
		lv_obj_add_flag(wf_kb, LV_OBJ_FLAG_HIDDEN);
}

static lv_obj_t *wf_btn(lv_obj_t *par, const char *t, lv_event_cb_t cb)
{
	lv_obj_t *b = lv_button_create(par);
	lv_obj_set_height(b, 56);
	lv_obj_set_style_bg_color(b, lv_color_hex(TH.btn), 0);
	lv_obj_set_style_bg_color(b, C_ACC, LV_STATE_PRESSED);
	lv_obj_set_style_text_color(b, lv_color_hex(TH.text), 0);	/* 0.22: Beyaz temada beyaz yazi okunmuyordu */
	lv_obj_t *l = label(b, f_mb, t);
	lv_obj_center(l);
	lv_obj_add_event_cb(b, cb, LV_EVENT_CLICKED, NULL);
	return b;
}

static void wf_open_cb(lv_event_t *e)
{
	if (wf_win)
		return;
	wf_win = lv_obj_create(lv_layer_top());
	lv_obj_remove_style_all(wf_win);
	lv_obj_set_size(wf_win, W, H);
	lv_obj_set_style_bg_opa(wf_win, LV_OPA_COVER, 0);
	lv_obj_set_style_bg_color(wf_win, C_BG, 0);
	lv_obj_set_style_text_color(wf_win, lv_color_hex(TH.text), 0);
	lv_obj_set_style_pad_all(wf_win, 12, 0);
	lv_obj_add_flag(wf_win, LV_OBJ_FLAG_CLICKABLE);

	lv_obj_t *hdr = lv_obj_create(wf_win);
	lv_obj_remove_style_all(hdr);
	lv_obj_set_size(hdr, LV_PCT(100), 60);
	lv_obj_set_flex_flow(hdr, LV_FLEX_FLOW_ROW);
	lv_obj_set_flex_align(hdr, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	label(hdr, f_l, LV_SYMBOL_WIFI "  Wi-Fi ağı");
	lv_obj_t *hb = lv_obj_create(hdr);
	lv_obj_remove_style_all(hb);
	lv_obj_set_size(hb, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
	lv_obj_set_flex_flow(hb, LV_FLEX_FLOW_ROW);
	lv_obj_set_style_pad_column(hb, 12, 0);
	wf_btn(hb, LV_SYMBOL_REFRESH "  Yenile", wf_rescan_cb);
	wf_btn(hb, LV_SYMBOL_CLOSE "  Kapat", wf_close_cb);

	lv_obj_t *row = lv_obj_create(wf_win);
	lv_obj_remove_style_all(row);
	lv_obj_set_size(row, LV_PCT(100), 220);
	lv_obj_set_y(row, 68);
	lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
	lv_obj_set_style_pad_column(row, 12, 0);
	wf_listobj = lv_list_create(row);
	lv_obj_set_size(wf_listobj, 720, LV_PCT(100));
	lv_obj_set_style_bg_color(wf_listobj, C_CARD, 0);
	lv_obj_set_style_border_width(wf_listobj, 0, 0);
	lv_obj_t *rp = card(row, NULL);
	wf_sel_l = label(rp, f_mb, "Bir ağ seç");
	wf_ta = lv_textarea_create(rp);
	lv_obj_set_width(wf_ta, LV_PCT(100));
	lv_textarea_set_one_line(wf_ta, true);
	lv_textarea_set_password_mode(wf_ta, true);
	lv_textarea_set_placeholder_text(wf_ta, "Şifre");
	lv_obj_set_style_text_font(wf_ta, f_m, 0);
	lv_obj_t *br = lv_obj_create(rp);
	lv_obj_remove_style_all(br);
	lv_obj_set_size(br, LV_PCT(100), LV_SIZE_CONTENT);
	lv_obj_set_flex_flow(br, LV_FLEX_FLOW_ROW);
	lv_obj_set_flex_align(br, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	wf_status = label(br, f_s, "");
	lv_label_set_long_mode(wf_status, LV_LABEL_LONG_WRAP);
	lv_obj_set_width(wf_status, LV_PCT(65));
	wf_btn(br, LV_SYMBOL_OK "  Bağlan", wf_connect_cb);

	wf_kb = lv_keyboard_create(wf_win);
	lv_obj_set_size(wf_kb, LV_PCT(100), 290);
	lv_obj_align(wf_kb, LV_ALIGN_BOTTOM_MID, 0, 0);
	lv_obj_set_style_text_font(wf_kb, f_mb, 0);
	lv_keyboard_set_textarea(wf_kb, wf_ta);
	lv_obj_add_event_cb(wf_kb, wf_kb_cb, LV_EVENT_ALL, NULL);
	lv_obj_add_flag(wf_kb, LV_OBJ_FLAG_HIDDEN);
	wf_drawn_state = -1;
	wf_start_scan();
}

/* zamanlayicidan: is parcaciginin sonucunu pencereye yansit */
static void wf_tick(void)
{
	if (!wf_win)
		return;
	pthread_mutex_lock(&mu);
	int st = wf_state;
	char msg[160];
	snprintf(msg, sizeof msg, "%s", wf_msg);
	wf_ag_t l[WF_MAX];
	int n = wf_n;
	memcpy(l, wf_list, sizeof l);
	pthread_mutex_unlock(&mu);
	lv_label_set_text(wf_status, msg);
	if (st == wf_drawn_state)
		return;
	wf_drawn_state = st;
	if (st == 2) {
		lv_obj_clean(wf_listobj);
		for (int i = 0; i < n; i++) {
			char t[100];
			snprintf(t, sizeof t, "%.63s   %d dBm%s", l[i].ssid, l[i].sig, l[i].sec ? "" : "  (açık)");	/* 0.22: sinirli (uyari) */
			lv_obj_t *b = lv_list_add_button(wf_listobj, LV_SYMBOL_WIFI, t);
			lv_obj_set_style_text_font(b, f_m, 0);
			lv_obj_set_height(b, 56);
			lv_obj_add_event_cb(b, wf_pick_cb, LV_EVENT_CLICKED, (void *)(uintptr_t)i);
		}
	}
}

static void br_mode_cb(lv_event_t *e)
{
	br_auto = lv_obj_has_state(sw_auto, LV_STATE_CHECKED);
	if (br_auto)
		lv_obj_add_flag(br_box, LV_OBJ_FLAG_HIDDEN);
	else
		lv_obj_remove_flag(br_box, LV_OBJ_FLAG_HIDDEN);
	br_save();
	br_apply();
}

static void br_slider_cb(lv_event_t *e)
{
	br_manual = lv_slider_get_value(sl_br);
	lv_label_set_text_fmt(l_br_pct, "%%%d", br_manual);
	br_apply();
	if (lv_event_get_code(e) == LV_EVENT_RELEASED)
		br_save();
}

/* ---------------------------------------------- kablosuz PC ekrani (0.14, pc_shm_v1.h) */
/* Alici (wifi-ekran-v4) JPEG'leri /tmp/pc-ekran.shm'deki 1600x600 RGB565 kareye cozer; panel ayni bellegi
 * kopyasiz LVGL goruntu kaynagi yapar ve yalniz bildirilen kirli dikdortgenleri yeniden cizer.
 * Tam ekran: ekranin ustunde 1600x600 goruntu; dokununca ust cubuk (panele don). Akis baslayinca (PC
 * "tam ekran iste" bayragiyla) otomatik acilir; kullanici kapattiysa ayni baglantida yeniden acilmaz. */
static void pc_bar_show(uint32_t ms);

#define PC_DG_DOSYA "/tmp/pc-dogrudan"
/* 0.18: {nesil, izin} yaz; izin degismediyse dokunma. izin=0 aninda sahiplik birakilmis sayilir. */
static void pc_dg_yaz(int izin)
{
	if (izin == pc_dg_izin && ekran_ters == pc_dg_ters)	/* 0.64: yon degisince de yaz */
		return;
	if (pc_dg_fd < 0) {
		/* silme/kesme yok: alicinin acik tanimlayicisi ayni dosyayi okumaya devam etsin */
		pc_dg_fd = open(PC_DG_DOSYA, O_RDWR | O_CREAT | O_CLOEXEC | O_NOFOLLOW, 0644);
		if (pc_dg_fd < 0)
			return;
	}
	pc_dg_gen++;
	uint32_t k[3] = { pc_dg_gen, (uint32_t)izin, (uint32_t)!!ekran_ters };	/* 0.64: 3. alan yon (alici v22) */
	int eski = pc_dg_izin;
	if (pwrite(pc_dg_fd, k, sizeof k, 0) == (ssize_t)sizeof k) {
		pc_dg_izin = izin;
		pc_dg_ters = ekran_ters;
	}
	if (!izin) {
		/* alici sahipken birakiliyorsa onay gelene kadar bekle (eski ortak bellek cizilmesin) */
		pc_dg_bekle = pc_rv_sahip || (eski == 1 && pc_shm.h && pc_shm.h->pad[1] == 1);
		pc_dg_t0 = tick_ms();
		pc_rv_sahip = 0;
	}
}

static void pc_full_exit(void)
{
	pc_dg_yaz(0);
	if (pc_full) {
		lv_obj_delete(pc_full);
		pc_full = pc_full_img = pc_bar = l_pc_bar = NULL;
		lv_timer_set_period(lv_display_get_refr_timer(disp), LV_DEF_REFR_PERIOD);
		lv_obj_invalidate(lv_screen_active());
	}
}

static void pc_full_back_cb(lv_event_t *e)
{
	(void)e;
	if (pc_shm.h) {
		pc_dismiss_gen = pc_shm.gen;
		pc_dismiss_conn = pc_shm.h->conn;
		pc_dismissed = 1;
	}
	pc_full_exit();
}

static void pc_ekran_iste_cb(lv_event_t *e)	/* 0.57: panel -> PC ajani "ikinci ekrani baslat" */
{
	(void)e;
	if (!pc_ajan_bagli() && !pc_ajan_var()) {	/* 0.58: state ile de */
		printf("ekran iste: PC ajani yok\n");
		show_toast(LV_SYMBOL_WARNING "  PC uygulaması bağlı değil: PC'de KÖLE uygulamasını açın");
		return;
	}
	send_event("{\"t\":\"ekran_iste\",\"ac\":true}");
	printf("ekran iste: PC'ye gonderildi\n");
	show_toast(LV_SYMBOL_UPLOAD "  PC'ye ekran isteği gönderildi");
}

static lv_obj_t *pc_ses_btn;	/* 0.58 */
static lv_obj_t *pc_bag_btn, *pc_kip_btn;	/* 0.65 */
static int pc_oto;	/* 0.65: 1 = PC Ekrani sekmesine girince otomatik baglan (<veri>/pc-baglan-oto) */

static void pc_ses_ciz(void)
{
	if (!pc_ses_btn)
		return;
	lv_obj_t *l = lv_obj_get_child(pc_ses_btn, 0);
	if (l)
		lv_label_set_text(l, !strcmp(pc_ses, "panel") ? LV_SYMBOL_VOLUME_MAX "  Ses: panel hoparlörü"
			: !strcmp(pc_ses, "pc") ? LV_SYMBOL_VOLUME_MAX "  Ses: PC hoparlörü" : LV_SYMBOL_VOLUME_MAX "  Ses: —");
}

static void pc_ses_cb(lv_event_t *e)	/* 0.58: ikinci ekran sesi panelden / PC'den (PC ajani v8) */
{
	(void)e;
	if (!pc_ajan_bagli()) {
		printf("ses yonu: PC ajani yok\n");
		show_toast(LV_SYMBOL_WARNING "  PC uygulaması bağlı değil: PC'de KÖLE uygulamasını açın");
		return;
	}
	const char *yeni = !strcmp(pc_ses, "panel") ? "pc" : "panel";
	char ev[64];
	snprintf(ev, sizeof ev, "{\"t\":\"ses_yonu\",\"mod\":\"%s\"}", yeni);
	send_event(ev);
	snprintf(pc_ses, sizeof pc_ses, "%s", yeni);
	pc_ses_ciz();
	printf("ses yonu: %s\n", yeni);
	show_toast(!strcmp(yeni, "panel") ? LV_SYMBOL_VOLUME_MAX "  İkinci ekran sesi panelden çıkacak"
		   : LV_SYMBOL_VOLUME_MAX "  İkinci ekran sesi PC'de kalacak");
}

static void pc_disconnect_cb(lv_event_t *e)
{
    (void)e;
    pc_session_disconnect(&pc_session);
    pc_full_back_cb(NULL);
    lv_tabview_set_active(tabview, 0, LV_ANIM_OFF);
}

/* 0.65: Baglan / Baglantiyi kes dugmesi ve Elle / Otomatik kip dugmesi */
static void pc_bag_ciz(void)
{
	static int son = -1;
	if (!pc_bag_btn || son == pc_session.want)
		return;
	son = pc_session.want;
	lv_obj_t *l = lv_obj_get_child(pc_bag_btn, 0);
	if (l)
		lv_label_set_text(l, pc_session.want ? LV_SYMBOL_CLOSE "  Bağlantıyı kes" : LV_SYMBOL_PLAY "  Bağlan");
}

static void pc_kip_ciz(void)
{
	lv_obj_t *l = pc_kip_btn ? lv_obj_get_child(pc_kip_btn, 0) : NULL;
	if (l)
		lv_label_set_text(l, pc_oto ? LV_SYMBOL_REFRESH "  Bağlanma: Otomatik" : LV_SYMBOL_REFRESH "  Bağlanma: Elle");
}

static void pc_kip_ayarla(int oto)
{
	char y[160];
	veri_yol(y, sizeof y, "pc-baglan-oto");
	pc_oto = !!oto;
	if (pc_oto) {
		int fd = open(y, O_WRONLY | O_CREAT | O_CLOEXEC, 0644);
		if (fd >= 0)
			close(fd);
	} else
		unlink(y);
	sync();
	pc_kip_ciz();
	printf("pc baglanma: %s\n", pc_oto ? "oto" : "elle");
}

static void pc_kip_cb(lv_event_t *e)
{
	(void)e;
	pc_kip_ayarla(!pc_oto);
	show_toast(pc_oto ? LV_SYMBOL_REFRESH "  PC Ekranı sekmesine gelince otomatik bağlanır"
		   : LV_SYMBOL_REFRESH "  Bağlanmak için Bağlan'a dokunun");
}

static void pc_bag_cb(lv_event_t *e)
{
	if (pc_session.want) {
		printf("pc baglan: kes\n");
		pc_disconnect_cb(e);
		pc_bag_ciz();
		return;
	}
	pc_session_connect(&pc_session);
	pc_bag_ciz();
	printf("pc baglan: %s\n", pc_oto ? "oto kipte elle" : "elle");
	show_toast(pc_ajan_bagli() ? LV_SYMBOL_PLAY "  PC ekranına bağlanılıyor…"
		   : LV_SYMBOL_WARNING "  PC uygulaması bağlı değil: PC'de KÖLE uygulamasını açın");
}

/* 0.15: tam ekranda dokunma -> PC faresi. Kayitlar FIFO /tmp/pc-dokunma'ya (alici wifi-ekran-v6 TCP ile PC'ye
 * iletir): 8 B = tip(1 bas, 2 surukle, 3 birak; 0.27: 4..9 iki parmak), 0, x(le16), y(le16), 0. 0.27: ust kenar
 * (y < 28) dokunusu PC'ye tiklama; "Panele don" cubugu yalniz kenardan asagi kaydirinca acilir. Alici yoksa yazma sessizce atlanir (O_NONBLOCK, SIGPIPE yok sayiliyor). */
static int pc_dok_fd = -1, pc_dok_kenar;
static uint32_t pc_dok_son, pc_dok_sayac;

static void pc_dok_yaz(int tip, int x, int y)
{
	if (x < 0) x = 0;
	if (y < 0) y = 0;
	if (x >= W) x = W - 1;
	if (y >= H) y = H - 1;
	if (pc_dok_fd < 0)
		pc_dok_fd = open("/tmp/pc-dokunma", O_WRONLY | O_NONBLOCK | O_CLOEXEC);
	if (pc_dok_fd < 0)
		return;
	uint8_t m[8] = { (uint8_t)tip, 0, (uint8_t)(x & 255), (uint8_t)(x >> 8), (uint8_t)(y & 255), (uint8_t)(y >> 8), 0, 0 };
	if (write(pc_dok_fd, m, sizeof m) == (ssize_t)sizeof m)
		pc_dok_sayac++;
	else if (errno != EAGAIN) {
		close(pc_dok_fd);	/* alici yeniden basladi (FIFO yenilendi): sonraki dokunusta yeniden ac */
		pc_dok_fd = -1;
	}
}

/* 0.27: iki parmak hareketleri (yalniz PC tam ekraninda). Kayit tipleri (alici v17, PC fare_v2): 4/5 teker
 * yukari/asagi, 6/7 yakinlastir/uzaklastir (Ctrl+teker), 8/9 yatay teker sola/saga; x,y = parmaklarin ortasi.
 * Tek parmak basisi JEST_BEKLE_MS bekletilir: ikinci parmak bu surede gelirse PC'ye hic tiklama gitmez.
 * Bir hareket once yakinlastirma ya da kaydirma olarak belirlenir, parmaklar kalkana kadar o turde kalir. */
#define JEST_BEKLE_MS 70
#define JEST_KAYDIR_PX 40
#define JEST_ORAN2 1.3225f	/* (1.15)^2: parmak arasi uzaklik %15 degisince bir yakinlastirma adimi */
#define KENAR_PX 28
#define KENAR_KAYDIR_PX 50
static int jest_kilit;	/* iki parmak goruldu: tum parmaklar kalkana kadar tek parmak olaylari PC'ye gitmez */
static int jest_tur;	/* 0 belirsiz, 1 kaydirma, 2 yakinlastirma */
static float jest_d20;	/* baslangic uzakligi^2 */
static int jest_cx0, jest_cy0;
static int pc_dok_bekle, pc_dok_basili, pc_dok_bx, pc_dok_by, pc_dok_sx, pc_dok_sy;
static uint32_t pc_dok_bt;

static void mt_ekran(int i, int *x, int *y)
{
	*x = (int)((int64_t)mt_x[i] * (W - 1) / ev_xmax);
	*y = (int)((int64_t)mt_y[i] * (H - 1) / ev_ymax);
}

static void pc_dok_bas_gonder(void)	/* bekleyen tek parmak basisini PC'ye gonder */
{
	pc_dok_bekle = 0;
	pc_dok_basili = 1;
	pc_dok_yaz(1, pc_dok_bx, pc_dok_by);
	pc_dok_son = tick_ms();
}

static void pc_jest_isle(void)
{
	if (!pc_full) {
		jest_kilit = pc_dok_bekle = pc_dok_basili = 0;
		return;
	}
	int n = 0, ya = -1, yb = -1;	/* 0.28: etkin parmak sayisi ve ilk iki etkin yuva */
	for (int k = 0; k < MT_YUVA_N; k++)
		if (mt_id[k] >= 0) {
			n++;
			if (ya < 0)
				ya = k;
			else if (yb < 0)
				yb = k;
		}
	if (n >= 2) {
		int x0, y0, x1, y1;
		mt_ekran(ya, &x0, &y0);
		mt_ekran(yb, &x1, &y1);
		int cx = (x0 + x1) / 2, cy = (y0 + y1) / 2;
		float d2 = (float)(x1 - x0) * (float)(x1 - x0) + (float)(y1 - y0) * (float)(y1 - y0);
		if (!jest_kilit) {
			jest_kilit = 1;
			jest_tur = 0;
			printf("iki parmak: yuva %d+%d (etkin %d)\n", ya, yb, n);
			if (pc_dok_bekle)
				pc_dok_bekle = 0;	/* bekleyen basis iptal: PC'ye tiklama gitmez */
			else if (pc_dok_basili) {
				pc_dok_yaz(3, pc_dok_sx, pc_dok_sy);
				pc_dok_basili = 0;
			}
			pc_dok_kenar = 0;
			jest_d20 = d2 > 1 ? d2 : 1;
			jest_cx0 = cx;
			jest_cy0 = cy;
			return;
		}
		if (jest_tur != 1 && (d2 > jest_d20 * JEST_ORAN2 || d2 * JEST_ORAN2 < jest_d20)) {
			pc_dok_yaz(d2 > jest_d20 ? 6 : 7, cx, cy);
			jest_tur = 2;
			jest_d20 = d2 > 1 ? d2 : 1;
			return;
		}
		if (jest_tur != 2) {
			int dy = cy - jest_cy0, dx = cx - jest_cx0;
			for (; dy >= JEST_KAYDIR_PX; dy -= JEST_KAYDIR_PX, jest_cy0 += JEST_KAYDIR_PX, jest_tur = 1)
				pc_dok_yaz(4, cx, cy);	/* parmaklar asagi: icerik asagi = yukari kaydir */
			for (; dy <= -JEST_KAYDIR_PX; dy += JEST_KAYDIR_PX, jest_cy0 -= JEST_KAYDIR_PX, jest_tur = 1)
				pc_dok_yaz(5, cx, cy);
			for (; dx >= JEST_KAYDIR_PX; dx -= JEST_KAYDIR_PX, jest_cx0 += JEST_KAYDIR_PX, jest_tur = 1)
				pc_dok_yaz(8, cx, cy);	/* parmaklar saga: icerik saga = sola kaydir */
			for (; dx <= -JEST_KAYDIR_PX; dx += JEST_KAYDIR_PX, jest_cx0 -= JEST_KAYDIR_PX, jest_tur = 1)
				pc_dok_yaz(9, cx, cy);
		}
		return;
	}
	if (jest_kilit) {
		if (n == 0 && !ev_down)
			jest_kilit = 0;
		return;
	}
	if (pc_dok_bekle && (uint32_t)(tick_ms() - pc_dok_bt) >= JEST_BEKLE_MS)
		pc_dok_bas_gonder();
}

static void pc_full_tap_cb(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_point_t p;
	lv_indev_t *in = lv_indev_active();
	if (!in)
		return;
	lv_indev_get_point(in, &p);
	uint32_t now = tick_ms();
	if (jest_kilit)
		return;	/* 0.27: iki parmak hareketi suruyor */
	if (code == LV_EVENT_PRESSED) {
		pc_dok_bx = pc_dok_sx = p.x;
		pc_dok_by = pc_dok_sy = p.y;
		pc_dok_bt = now;
		pc_dok_basili = 0;
		pc_dok_kenar = p.y < KENAR_PX;	/* 0.27: kenar = karar bekliyor (asagi kaydirma mi, tiklama mi) */
		pc_dok_bekle = !pc_dok_kenar;
	} else if (code == LV_EVENT_PRESSING) {
		pc_dok_sx = p.x;
		pc_dok_sy = p.y;
		if (pc_dok_kenar == 1) {
			if (p.y - pc_dok_by >= KENAR_KAYDIR_PX) {
				pc_dok_kenar = 2;	/* asagi kaydirildi: cubuk, PC'ye hicbir sey gitmez */
				pc_bar_show(5000);
				return;
			}
			if (LV_ABS(p.x - pc_dok_bx) > 30 || (uint32_t)(now - pc_dok_bt) > 400) {
				pc_dok_kenar = 0;	/* yana surukleme ya da uzun basis: normal dokunma */
				pc_dok_bas_gonder();
			}
			return;
		}
		if (pc_dok_kenar)
			return;
		if (pc_dok_bekle && (LV_ABS(p.x - pc_dok_bx) > 8 || LV_ABS(p.y - pc_dok_by) > 8))
			pc_dok_bas_gonder();	/* surukleme basladi: beklemeden bas */
		if (pc_dok_basili && (uint32_t)(now - pc_dok_son) >= 30) {
			pc_dok_yaz(2, p.x, p.y);
			pc_dok_son = now;
		}
	} else if (code == LV_EVENT_RELEASED) {
		if (pc_dok_kenar == 2) {
			pc_dok_kenar = 0;
			return;
		}
		if (pc_dok_kenar == 1 || pc_dok_bekle) {	/* kisa dokunus: bas + birak (tiklama) */
			pc_dok_yaz(1, pc_dok_bx, pc_dok_by);
			pc_dok_yaz(3, pc_dok_bx, pc_dok_by);
		} else if (pc_dok_basili) {
			pc_dok_yaz(3, p.x, p.y);
		}
		pc_dok_kenar = pc_dok_bekle = pc_dok_basili = 0;
	}
}

static void pc_bar_show(uint32_t ms)
{
	if (!pc_bar)
		return;
	pc_dg_yaz(0);	/* cubuk LVGL ile cizilecek: fb'yi geri al */
	lv_obj_remove_flag(pc_bar, LV_OBJ_FLAG_HIDDEN);
	pc_bar_until = tick_ms() + ms;
}

static void pc_full_enter(void)
{
	if (pc_full || !pc_session.want || !pc_shm.h || !pc_shm_alive(&pc_shm, tick_ms()))
		return;
	if (ss_obj)
		ss_hide();
	dm_git(0, 0);	/* 0.23: Denetim Merkezi aciksa kapat (PC goruntusu ustte olmali) */
	lv_display_trigger_activity(disp);
	pc_full = lv_obj_create(lv_screen_active());
	lv_obj_remove_style_all(pc_full);	/* arka plan saydam: goruntu tamamen ortuyor, cift cizim yok */
	lv_obj_set_size(pc_full, W, H);
	lv_obj_set_pos(pc_full, 0, 0);
	lv_obj_add_flag(pc_full, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_remove_flag(pc_full, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_add_event_cb(pc_full, pc_full_tap_cb, LV_EVENT_PRESSED, NULL);
	lv_obj_add_event_cb(pc_full, pc_full_tap_cb, LV_EVENT_PRESSING, NULL);
	lv_obj_add_event_cb(pc_full, pc_full_tap_cb, LV_EVENT_RELEASED, NULL);
	pc_full_img = lv_image_create(pc_full);
	lv_image_set_src(pc_full_img, &pc_dsc);
	lv_obj_set_pos(pc_full_img, 0, 0);
	pc_bar = lv_obj_create(pc_full);
	lv_obj_remove_style_all(pc_bar);
	lv_obj_set_size(pc_bar, W, 64);
	lv_obj_set_style_bg_color(pc_bar, lv_color_black(), 0);
	lv_obj_set_style_bg_opa(pc_bar, LV_OPA_70, 0);
	lv_obj_set_style_pad_hor(pc_bar, 16, 0);
	lv_obj_remove_flag(pc_bar, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_add_flag(pc_bar, LV_OBJ_FLAG_CLICKABLE);	/* cubuga dokunus PC'ye gitmez */
	lv_obj_t *b = lv_button_create(pc_bar);
	lv_obj_set_size(b, 220, 48);
	lv_obj_align(b, LV_ALIGN_LEFT_MID, 0, 0);
	lv_obj_add_event_cb(b, pc_full_back_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_t *bl = label(b, f_mb, LV_SYMBOL_LEFT "  Panele dön");
	lv_obj_center(bl);
    lv_obj_t *close = lv_button_create(pc_bar);
    lv_obj_set_size(close, 260, 48);
    lv_obj_align(close, LV_ALIGN_LEFT_MID, 236, 0);
    lv_obj_add_event_cb(close, pc_disconnect_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_t *cl = label(close, f_mb, "Bağlantıyı kes"); lv_obj_center(cl);
	l_pc_bar = label(pc_bar, f_s, "");
	lv_obj_set_style_text_color(l_pc_bar, lv_color_white(), 0);
	lv_obj_align(l_pc_bar, LV_ALIGN_RIGHT_MID, 0, 0);
	pc_bar_show(3000);
	lv_timer_set_period(lv_display_get_refr_timer(disp), 15);
}

static void pc_full_btn_cb(lv_event_t *e)
{
	(void)e;
	pc_session_connect(&pc_session);
	pc_dismissed = 0;
	pc_full_enter();
}

static void pc_attach(void)
{
	pc_dsc = (lv_image_dsc_t){ .header = { .magic = LV_IMAGE_HEADER_MAGIC, .cf = LV_COLOR_FORMAT_RGB565,
					       .w = PC_SHM_W, .h = PC_SHM_H, .stride = PC_SHM_W * 2 },
				   .data_size = PC_SHM_W * PC_SHM_H * 2, .data = pc_shm.px };
	lv_image_cache_drop(&pc_dsc);
	if (pc_image) {
		lv_image_set_src(pc_image, &pc_dsc);
		lv_obj_remove_flag(pc_image, LV_OBJ_FLAG_HIDDEN);
	}
	printf("PC ekrani: ortak bellek baglandi (nesil %u)\n", pc_shm.gen);
}

/* Esleme kaldirilmadan ONCE butun LVGL baglantilari kesilir (silinmis bellegi cizme riski yok). */
static void pc_detach(void)
{
	pc_dg_yaz(0);
	pc_full_exit();
	if (pc_image) {
		lv_image_set_src(pc_image, NULL);
		lv_obj_add_flag(pc_image, LV_OBJ_FLAG_HIDDEN);
	}
	lv_image_cache_drop(&pc_dsc);
	pc_shm_close(&pc_shm);
	printf("PC ekrani: ortak bellek birakildi\n");
}

/* 0.17: RGB565 (ortak bellek) -> XRGB8888 (fb0), bit cogaltmali (565 -> 888 tam aralik). */
static inline uint32_t pc_565_8888(uint32_t p)
{
	return 0xFF000000u | ((p & 0xF800u) << 8) | ((p & 0xE000u) << 3) | ((p & 0x07E0u) << 5) |
	       ((p & 0x0600u) >> 1) | ((p & 0x1Fu) << 3) | ((p & 0x1Cu) >> 2);
}

static void pc_dc_rect(uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
	const uint16_t *src = (const uint16_t *)pc_shm.px;
	if (!fb_rot) {
		for (uint32_t j = 0; j < h; j++) {
			const uint16_t *s = src + (size_t)(y + j) * PC_SHM_W + x;
			uint32_t *d = (uint32_t *)(fbm + (size_t)(y + j) * fb_line) + x;
			uint32_t i = 0;
			if (((uintptr_t)d & 7u) && w) {
				d[0] = pc_565_8888(s[0]);
				i = 1;
			}
			uint64_t *d64 = (uint64_t *)(d + i);
			for (; i + 1 < w; i += 2)
				*d64++ = (uint64_t)pc_565_8888(s[i]) | (uint64_t)pc_565_8888(s[i + 1]) << 32;
			if (i < w)
				d[i] = pc_565_8888(s[i]);
		}
	} else {	/* yazilim 90 derece (G2D'siz cekirdek): mantiksal (x,y) -> fiziksel (xres-1-y, x) */
		for (uint32_t j = 0; j < h; j++) {
			const uint16_t *s = src + (size_t)(y + j) * PC_SHM_W + x;
			uint32_t col = ekran_ters ? y + j : fb_xres - 1 - (y + j);	/* 0.47 */
			for (uint32_t i = 0; i < w; i++)
				((uint32_t *)(fbm + (size_t)(ekran_ters ? fb_yres - 1 - (x + i) : x + i) * fb_line))[col] = pc_565_8888(s[i]);
		}
	}
	pc_dc_px += (uint64_t)w * h;
}

static int pc_katman_gorunur(lv_obj_t *k)
{
	uint32_t n = lv_obj_get_child_count(k);
	for (uint32_t i = 0; i < n; i++)
		if (!lv_obj_has_flag(lv_obj_get_child(k, (int32_t)i), LV_OBJ_FLAG_HIDDEN))
			return 1;
	return 0;
}

/* 0.17-v3: 16x16 karo haritasi; her karo bir kez cizilir, ayni satirdaki bitisik karolar tek parca. */
#define PC_KARO 16u
#define PC_KX (PC_SHM_W / PC_KARO)
#define PC_KY ((PC_SHM_H + PC_KARO - 1) / PC_KARO)
static uint8_t pc_karo[PC_KY][PC_KX];

static void pc_dc_karolar(uint16_t r[][4], int n)
{
	memset(pc_karo, 0, sizeof pc_karo);
	for (int i = 0; i < n; i++) {
		uint32_t x0 = r[i][0] / PC_KARO, y0 = r[i][1] / PC_KARO;
		uint32_t x1 = (r[i][0] + r[i][2] - 1u) / PC_KARO, y1 = (r[i][1] + r[i][3] - 1u) / PC_KARO;
		if (x1 >= PC_KX)
			x1 = PC_KX - 1;
		for (uint32_t ty = y0; ty <= y1 && ty < PC_KY; ty++)
			memset(&pc_karo[ty][x0], 1, x1 - x0 + 1);
	}
	for (uint32_t ty = 0; ty < PC_KY; ty++) {
		uint32_t y = ty * PC_KARO, h = y + PC_KARO > PC_SHM_H ? PC_SHM_H - y : PC_KARO;
		for (uint32_t tx = 0; tx < PC_KX;) {
			if (!pc_karo[ty][tx]) {
				tx++;
				continue;
			}
			uint32_t b = tx;
			while (tx < PC_KX && pc_karo[ty][tx])
				tx++;
			pc_dc_rect(b * PC_KARO, y, (tx - b) * PC_KARO, h);
		}
	}
}

/* 1: dogrudan cizildi; 0: LVGL yolu kullanilmali. n < 0: tam ekran. */
static int pc_dc_ciz(uint16_t r[][4], int n)
{
	if (!pc_dc_izin || !pc_full_img || !pc_shm.h || !fbm)
		return 0;
	if (pc_bar && !lv_obj_has_flag(pc_bar, LV_OBJ_FLAG_HIDDEN))
		return 0;
	if (pc_katman_gorunur(lv_layer_top()) || pc_katman_gorunur(lv_layer_sys()))
		return 0;	/* bildirim/ust katman acik: LVGL ciziyor */
	struct timespec t0, t1;
	clock_gettime(CLOCK_MONOTONIC, &t0);
	if (n < 0)
		pc_dc_rect(0, 0, PC_SHM_W, PC_SHM_H);
	else
		pc_dc_karolar(r, n);
	{
		struct timespec tc;
		clock_gettime(CLOCK_MONOTONIC, &tc);
		pc_dc_cv_us += elapsed_us(&t0, &tc);
	}
	if (fb_hw_pan) {
		struct timespec p0, p1;
		clock_gettime(CLOCK_MONOTONIC, &p0);
		int rr = ioctl(fb_fd, FBIOPAN_DISPLAY, &fb_var);
		clock_gettime(CLOCK_MONOTONIC, &p1);
		pc_pan_calls++;
		pc_pan_us += elapsed_us(&p0, &p1);
		if (rr < 0) {
			pc_pan_errors++;
			if (pc_pan_errors <= 2) perror("G2D FBIOPAN_DISPLAY (dogrudan)");
		}
	}
	clock_gettime(CLOCK_MONOTONIC, &t1);
	pc_dc_n++;
	pc_dc_us += elapsed_us(&t0, &t1);
	return 1;
}

/* 0.26: alici bayragi istekle esit; yalniz durum degisince dosyaya dokunulur (bir kez de acilista) */
#define PC_ALICI_BAYRAK "/tmp/ikinci-ekran-acik"
static void pc_alici_esitle(int want)
{
	static int son = -1;
	if (want == son)
		return;
	if (want) {
		int fd = open(PC_ALICI_BAYRAK, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
		if (fd < 0)
			return;	/* son degismez: bir sonraki tikte yeniden denenir */
		close(fd);
	} else if (unlink(PC_ALICI_BAYRAK) && errno != ENOENT) {
		return;
	}
	son = want;
	printf("pc alici: %s\n", want ? "istendi (bayrak)" : "birakildi");
}

static void pc_image_tick(lv_timer_t *timer)
{
	if (!pc_conn && br_pc > 0) {	/* 0.58: parlaklik veren PC ajani koptu -> panelin kendi gece/gunduz plani */
		br_pc = -1;
		br_apply();
		printf("PC parlakligi birakildi (baglanti yok): panel plani\n");
	}
	if (timer) {	/* 0.59: PC ile ilgili bir sey yokken 100 ms (ana dongu ~100 -> ~33 uyanma/sn) */
		static uint32_t per = 10;
		uint32_t ist = (pc_shm.h || pc_full || pc_session.want ||
				(tabview && lv_tabview_get_tab_active(tabview) == S_PC)) ? 10 : 100;
		if (ist != per) {
			per = ist;
			lv_timer_set_period(timer, per);
			printf("pc tick %u ms\n", (unsigned)per);
		}
	}
	static uint32_t t_try, t_check, t_prev, t_label, t_report;
	uint32_t now = tick_ms();
	if (!pc_shm.h) {
		if ((uint32_t)(now - t_try) >= 500) {
			t_try = now;
			if (pc_shm_open(&pc_shm, PC_SHM_PATH) == 1)
				pc_attach();
		}
	} else if ((uint32_t)(now - t_check) >= 500) {
		t_check = now;
		if (!pc_shm_still_current(&pc_shm, PC_SHM_PATH))
			pc_detach();
	}
	int alive = pc_shm.h && pc_shm_alive(&pc_shm, now);
	int tab5 = tabview && lv_tabview_get_tab_active(tabview) == S_PC;	/* 0.23: PC sekmesi artik S_PC */
	pc_session_tick(&pc_session, tab5, !!pc_full, pc_oto);	/* 0.65: Elle kipte sekmeye girmek baglamaz */
	pc_alici_esitle(!!pc_session.want);	/* 0.26 */
	pc_view_active = pc_full || tab5;
	if (alive) {
		pc_last_alive = now;
		if (pc_dismissed && (pc_dismiss_gen != pc_shm.gen || pc_dismiss_conn != pc_shm.h->conn))
			pc_dismissed = 0;
		if (pc_session.want && pc_shm.h->want_full && !pc_full && !pc_dismissed && !kilitli)	/* 0.23: kilitte otomatik acma */
			pc_full_enter();
	}
	if (pc_dg_acik) {
		/* 0.51: wifi-ekran v21 dogrudan cizimi sabit ROT_90 (panel <=0.46 yonu); 180 derecede devir yok.
		 * 0.64: alici v22 pad[10] ile 180 destegini bildirirse 180 derecede de devir (ROT_270) */
		int ist = (!ekran_ters || (pc_shm.h && (pc_shm.h->pad[10] & 1))) && pc_dc_izin && pc_full_img && pc_shm.h && fbm && (fb_hw_pan || fb_rot) && alive &&
			  !(pc_bar && !lv_obj_has_flag(pc_bar, LV_OBJ_FLAG_HIDDEN)) &&
			  !pc_katman_gorunur(lv_layer_top()) && !pc_katman_gorunur(lv_layer_sys());
		pc_dg_yaz(ist);
		pc_rv_sahip = ist && pc_dg_izin == 1 && pc_shm.h->pad[0] == pc_dg_gen && pc_shm.h->pad[1] == 1;
		if (pc_dg_bekle && (!pc_shm.h || (pc_shm.h->pad[0] == pc_dg_gen && pc_shm.h->pad[1] == 0) ||
				    (uint32_t)(now - pc_dg_t0) > 400)) {
			pc_dg_bekle = 0;
			pc_dg_bekleme++;
			if (pc_full_img)
				lv_obj_invalidate(pc_full_img);	/* esitlenmis ortak bellekten tam yeniden ciz */
			else {	/* 0.71: tam ekrandan cikildi: beklerken atlanan cizimler (ust serit, alt menu, sayfa) -> tum ekran */
				lv_obj_invalidate(lv_screen_active());
				lv_obj_invalidate(lv_layer_top());
				lv_obj_invalidate(lv_layer_sys());
				printf("pc devir bitti: tam ekran yeniden ciziliyor (%u atlanan flush, %u ms)\n", (unsigned)pc_dg_atlanan,
				       (unsigned)(now - pc_dg_t0));
			}
			pc_dg_atlanan = 0;
		}
	}
	if (pc_shm.h) {
		uint16_t r[32][4];
		int n = pc_shm_take(&pc_shm, r, 32);
		if (n && (pc_rv_sahip || pc_dg_bekle)) {
			pc_updates++;
			pc_rv_rects += n > 0 ? (uint32_t)n : 1u;
		} else if (n) {
			pc_updates++;
			if (pc_full_img && pc_dc_ciz(r, n)) {
				if (n < 0)
					pc_inv_full++;
				else
					pc_inv_rects += (uint32_t)n;
			} else if (pc_full_img) {
				if (n < 0) {
					lv_obj_invalidate(pc_full_img);
					pc_inv_full++;
				} else
					for (int i = 0; i < n; i++) {
						lv_area_t a = { r[i][0], r[i][1], r[i][0] + r[i][2] - 1, r[i][1] + r[i][3] - 1 };
						lv_obj_invalidate_area(pc_full_img, &a);
						pc_inv_rects++;
					}
			} else if (tab5 && pc_image && (uint32_t)(now - t_prev) >= 250) {
				t_prev = now;	/* olcekli onizleme pahali: en fazla 4 Hz */
				lv_obj_invalidate(pc_image);
			}
		}
	}
	if (pc_full) {
		if (alive)
			lv_display_trigger_activity(disp);	/* izlerken ekran koruyucu acilmasin */
		else {
			pc_bar_show(1000);
			if ((uint32_t)(now - pc_last_alive) > 10000)
				pc_full_exit();	/* akis 10 sn durdu: panele don */
		}
		if (pc_bar && (int32_t)(now - pc_bar_until) > 0 && alive)
			lv_obj_add_flag(pc_bar, LV_OBJ_FLAG_HIDDEN);
	}
	if ((uint32_t)(now - t_label) >= 1000) {
		t_label = now;
		char s[160];
		pc_bag_ciz();	/* 0.65 */
		if (!pc_session.want)
			snprintf(s, sizeof s, "%s", pc_oto ? "PC ekranı kapalı · Bağlanmak için PC Ekranı sekmesini aç"
						     : "PC ekranı bağlı değil · Bağlan'a dokunun (Bağlanma: Elle)");
		else if (!alive)
			snprintf(s, sizeof s, "%s", pc_status);
		else if (!pc_shm.h)
			snprintf(s, sizeof s, "PC görüntüsü bekleniyor (alıcı kapalı)");
		else if (!alive)
			snprintf(s, sizeof s, "%s", pc_shm.h->frames ? "Görüntü akışı durdu" : "Alıcı hazır, PC bekleniyor");
		else
			snprintf(s, sizeof s, "Kablosuz · 1600 × 600 · %u.%u fps · %u kbit/s · çözme %u ms",
				 pc_shm.h->fps_x10 / 10, pc_shm.h->fps_x10 % 10, pc_shm.h->kbps, pc_shm.h->last_decode_us / 1000);
		if (l_pc_image)
			lv_label_set_text(l_pc_image, s);
		if (l_pc_bar)
			lv_label_set_text(l_pc_bar, s);
	}
	if ((uint32_t)(now - t_report) >= 1000) {
		t_report = now;
        if (pc_conn) {
            char event[128];
            snprintf(event, sizeof event, "{\"t\":\"display\",\"schema\":1,\"want\":%d,\"seq\":%u}", pc_session.want, ++pc_request_seq);
            send_event(event);
        }
		FILE *f = fopen("/tmp/pc-ekran-durum.json.part", "w");
		if (f) {
			fprintf(f, "{\"ver\":\"%s\",\"requested\":%d,\"attached\":%d,\"alive\":%d,\"full\":%d,\"tab5\":%d,\"conn\":%u,"
				"\"rx_frames\":%u,\"rx_errors\":%u,\"fps_x10\":%u,\"kbps\":%u,\"decode_us\":%u,"
				"\"updates\":%u,\"inv_rects\":%u,\"inv_full\":%u,\"render_n\":%u,\"render_avg_us\":%u,"
				"\"render_max_us\":%u,\"flush_us\":%llu,\"flush_calls\":%llu,\"touches\":%u,"
				"\"g2d_pan\":%d,\"pan_us\":%llu,\"pan_calls\":%llu,\"pan_errors\":%llu,"
				"\"direct\":%d,\"dc_n\":%llu,\"dc_us\":%llu,\"dc_px\":%llu,\"dc_cv_us\":%llu,"
				"\"dg_izin\":%d,\"rv_sahip\":%d,\"dg_bekleme\":%u,\"rv_rects\":%u,\"rv_g2d_n\":%u,\"rv_g2d_us\":%u,"
				"\"rv_pan_n\":%u,\"rv_pan_us\":%u,\"rv_cpu\":%u,\"rv_esit\":%u}\n",
				PANEL_VER, pc_session.want, !!pc_shm.h, alive, !!pc_full, tab5, pc_shm.h ? pc_shm.h->conn : 0,
				pc_shm.h ? pc_shm.h->frames : 0, pc_shm.h ? pc_shm.h->errors : 0,
				pc_shm.h ? pc_shm.h->fps_x10 : 0, pc_shm.h ? pc_shm.h->kbps : 0,
				pc_shm.h ? pc_shm.h->last_decode_us : 0, pc_updates, pc_inv_rects, pc_inv_full, pc_render_n,
				pc_render_n ? (uint32_t)(pc_render_sum / pc_render_n) : 0, pc_render_max,
				(unsigned long long)pc_flush_us, (unsigned long long)pc_flush_calls, pc_dok_sayac, fb_hw_pan,
				(unsigned long long)pc_pan_us, (unsigned long long)pc_pan_calls, (unsigned long long)pc_pan_errors,
				pc_dc_izin, (unsigned long long)pc_dc_n, (unsigned long long)pc_dc_us, (unsigned long long)pc_dc_px, (unsigned long long)pc_dc_cv_us,
				pc_dg_izin, pc_rv_sahip, pc_dg_bekleme, pc_rv_rects,
				pc_shm.h ? pc_shm.h->pad[2] : 0, pc_shm.h ? pc_shm.h->pad[3] : 0, pc_shm.h ? pc_shm.h->pad[4] : 0,
				pc_shm.h ? pc_shm.h->pad[5] : 0, pc_shm.h ? pc_shm.h->pad[6] : 0, pc_shm.h ? pc_shm.h->pad[7] : 0);
			fclose(f);
			rename("/tmp/pc-ekran-durum.json.part", "/tmp/pc-ekran-durum.json");
		}
		pc_render_max = 0;	/* en yuksek deger son 1 sn icin */
	}
}

/* ------------------------------------------------------- 0.22 gizlilik sekmesi */
/* Kamera durumu YALNIZ okunur (/tmp/kamera-kayit.json, kamera-kayit-v1 her saniye yazar). Panelin yapabildigi tek
 * kamera islemi KAPATMAKTIR: kole-kopru v6 "KAYIT 0" (/mnt/sd/kamera/kayit-acik siler) ve "KAMERA 0"
 * (/tmp/kamera-aktif siler) ile ayni. Kamera panelden ACILAMAZ (KOLE Merkez parola + sure ister). Kayitli dosyalara
 * dokunulmaz. Mikrofon yalniz gosterge: /tmp/mik-aktif'i silmek akisi durdurmaz, gostergeyi yanlis sondururdu. */
#define GZ_KAMERA_BAYRAK "/tmp/kamera-aktif"
static const char *kk_kok(void);	/* asagida (PANEL_KAMERA_KOK) */
static const char *gz_kayit_yol(void)	/* 0.57: kamera kokune bagli (host provasi PANEL_KAMERA_KOK) */
{
	static char y[300];
	snprintf(y, sizeof y, "%.280s/kayit-acik", kk_kok());
	return y;
}
#define GZ_KAYIT_BAYRAK gz_kayit_yol()
#define GZ_MIK_BAYRAK "/tmp/mik-aktif"
#define GZ_KALKAN_BAYRAK "/tmp/gizlilik-kalkani"
#define GZ_DURUM "/tmp/kamera-kayit.json"
#define GZ_GUNLUK "/tmp/panel-gizlilik.log"
#define GZ_SEKME S_GIZLILIK	/* 0.23 */

static lv_obj_t *gz_kam_l, *gz_kam_alt, *gz_v_canli, *gz_v_kayit, *gz_v_fps, *gz_v_parca, *gz_v_toplam, *gz_v_bos,
	*gz_v_son, *gz_v_hata, *gz_mik_l, *gz_btn, *gz_btn_l, *gz_sw, *gz_son, *gz_mb;
static char gz_son_islem[96] = "Henüz işlem yapılmadı.";

static int dosya_var(const char *y)
{
	struct stat st;
	return !stat(y, &st);
}

static void gz_gunluk(const char *olay)
{
	time_t now = time(NULL);
	struct tm tm;
	char z[32];
	localtime_r(&now, &tm);
	strftime(z, sizeof z, "%Y-%m-%d %H:%M:%S", &tm);
	FILE *f = fopen(GZ_GUNLUK, "a");
	if (f) {
		fprintf(f, "%s %s\n", z, olay);
		fclose(f);
	}
	snprintf(gz_son_islem, sizeof gz_son_islem, "%02d:%02d  %s", tm.tm_hour, tm.tm_min, olay);
}

static void gz_tekrar_cb(lv_timer_t *t)
{
	(void)t;
	/* kamera-kayit dongusu kayit bayragini silmeden hemen once okuduysa kamera bayragini yeniden yaratmis olabilir */
	if (!dosya_var(GZ_KAYIT_BAYRAK) && dosya_var(GZ_KAMERA_BAYRAK))
		unlink(GZ_KAMERA_BAYRAK);
}

/* 0: kapatildi (ya da zaten kapali), -1: bir bayrak silinemedi */
static int gz_kamera_kapat(const char *neden)
{
	int r = 0, vardi = dosya_var(GZ_KAMERA_BAYRAK) || dosya_var(GZ_KAYIT_BAYRAK);
	if (unlink(GZ_KAYIT_BAYRAK) && errno != ENOENT)
		r = -1;	/* once kayit: yoksa kamera-kayit /tmp/kamera-aktif'i geri yaratir */
	if (unlink(GZ_KAMERA_BAYRAK) && errno != ENOENT)
		r = -1;
	static time_t son_hata;
	if (r && vardi) {	/* silinemiyorsa (SD salt okunur) kalkan her saniye denemesin diye: gunluk/olay dakikada bir */
		time_t sn = mono_s();
		if (son_hata && sn - son_hata < 60)
			vardi = 0;
		else
			son_hata = sn;
	}
	if (vardi) {
		lv_timer_t *t = lv_timer_create(gz_tekrar_cb, 1500, NULL);
		lv_timer_set_repeat_count(t, 1);	/* bir kez calisir, LVGL kendisi siler */
		char ev[160];
		gz_gunluk(r ? "kamera kapatma: SD kayıt bayrağı silinemedi" : neden);
		snprintf(ev, sizeof ev, "{\"t\":\"privacy\",\"camera\":\"off\",\"ok\":%s,\"by\":\"panel\"}", r ? "false" : "true");
		send_event(ev);	/* PC ajani bilinmeyen turu yok sayar; ileride Merkez denetim gunlugune */
	}
	return r;
}

/* 0.25: SD kaydini panelden baslat (parolasiz; dokunmatik = cihazin basinda). 0 basladi/zaten acik, -1 olmadi */
static int gz_kayit_baslat(void)
{
	if (g_kalkan) {
		show_toast(LV_SYMBOL_EYE_CLOSE "  Gizlilik kalkanı açık: önce kalkanı kapatın");
		return -1;
	}
	if (dosya_var(GZ_KAYIT_BAYRAK))
		return 0;
	mkdir(kk_kok(), 0755);	/* 0.57 */
	int fd = open(GZ_KAYIT_BAYRAK, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
	if (fd >= 0)
		close(fd);
	sync();
	int ok = dosya_var(GZ_KAYIT_BAYRAK);
	g_kayit = ok;
	gz_gunluk(ok ? "SD kaydı panelden başlatıldı" : "SD kaydı başlatılamadı (SD takılı/yazılabilir mi?)");
	send_event(ok ? "{\"t\":\"privacy\",\"camera\":\"rec\",\"ok\":true,\"by\":\"panel\"}"
		      : "{\"t\":\"privacy\",\"camera\":\"rec\",\"ok\":false,\"by\":\"panel\"}");
	show_toast(ok ? LV_SYMBOL_VIDEO "  SD kaydı başladı (2 kare/sn, en çok 6 saatlik döngü)"	/* 0.79: SD dolarsa kisalir (B4) */
		      : LV_SYMBOL_WARNING "  SD kaydı başlatılamadı (SD kart?)");
	return ok ? 0 : -1;
}

static void gz_mb_kapat(void)
{
	if (gz_mb) {
		lv_msgbox_close_async(gz_mb);	/* perdesiyle birlikte; kendi dugmesinin olayi icinden: async */
		gz_mb = NULL;
	}
}

static void gz_onay_cb(lv_event_t *e)
{
	int evet = (int)(uintptr_t)lv_event_get_user_data(e);
	gz_mb_kapat();
	if (!evet)
		return;
	if (gz_kamera_kapat("kamera panelden kapatıldı (canlı + SD kaydı)"))
		show_toast(LV_SYMBOL_WARNING "  SD kaydı durdurulamadı (SD salt okunur mu?)");
	else
		show_toast(LV_SYMBOL_EYE_CLOSE "  Kamera kapatıldı");
}

static lv_obj_t *gz_mb_btn(lv_obj_t *mb, const char *t, int evet)
{
	lv_obj_t *b = lv_msgbox_add_footer_button(mb, t);
	lv_obj_set_height(b, 64);
	lv_obj_set_width(b, 240);
	lv_obj_set_style_radius(b, 14, 0);
	lv_obj_set_style_text_font(b, f_mb, 0);
	lv_obj_set_style_bg_color(b, evet ? lv_color_hex(0xDC2626) : lv_color_hex(TH.btn), 0);
	lv_obj_set_style_text_color(b, evet ? lv_color_white() : lv_color_hex(TH.text), 0);
	lv_obj_add_event_cb(b, gz_onay_cb, LV_EVENT_CLICKED, (void *)(uintptr_t)evet);
	return b;
}

static void gz_btn_cb(lv_event_t *e)
{
	(void)e;
	if (gz_mb)
		return;
	if (!(g_kam || g_kayit)) {	/* 0.25: kapaliyken ayni dugme kaydi baslatir (onaysiz) */
		gz_kayit_baslat();
		return;
	}
	gz_mb = lv_msgbox_create(NULL);
	lv_obj_set_width(gz_mb, 900);
	lv_obj_set_style_radius(gz_mb, 20, 0);
	lv_obj_set_style_bg_color(gz_mb, C_CARD, 0);
	lv_obj_set_style_border_width(gz_mb, 1, 0);
	lv_obj_set_style_border_color(gz_mb, lv_color_hex(TH.sep), 0);
	lv_obj_set_style_shadow_width(gz_mb, 0, 0);
	lv_obj_set_style_text_color(gz_mb, lv_color_hex(TH.text), 0);
	lv_obj_set_style_pad_all(gz_mb, 24, 0);
	lv_obj_set_style_pad_row(gz_mb, 14, 0);
	lv_obj_t *t = lv_msgbox_add_title(gz_mb, LV_SYMBOL_EYE_CLOSE "  Kamera kapatılsın mı?");
	lv_obj_set_style_text_font(t, f_l, 0);
	lv_obj_t *h = lv_msgbox_get_header(gz_mb);	/* baslik eklenince olusur */
	if (h) {
		lv_obj_set_style_bg_opa(h, LV_OPA_TRANSP, 0);
		lv_obj_set_height(h, LV_SIZE_CONTENT);
	}
	lv_obj_t *x = lv_msgbox_add_text(gz_mb, "Canlı izleme ve SD kartına kayıt durdurulur; kaydedilmiş dosyalar "
					   "silinmez.\nYeniden başlatmak için bu düğmeye ya da Denetim Merkezi'nde Kamera'ya dokunun.");
	lv_obj_set_style_text_font(x, f_m, 0);
	lv_obj_set_style_text_line_space(x, 6, 0);
	lv_obj_set_style_text_color(x, C_DIM, 0);
	gz_mb_btn(gz_mb, "Vazgeç", 0);
	gz_mb_btn(gz_mb, LV_SYMBOL_POWER "  Kapat", 1);
	lv_obj_t *ft = lv_msgbox_get_footer(gz_mb);	/* alt kisim ilk dugmeyle olusur: once dugmeler */
	if (ft) {
		lv_obj_set_height(ft, LV_SIZE_CONTENT);
		lv_obj_set_style_bg_opa(ft, LV_OPA_TRANSP, 0);
		lv_obj_set_style_pad_top(ft, 10, 0);
		lv_obj_set_style_pad_column(ft, 16, 0);
	}
}

static void ayar_esitle(void);	/* 0.23: tum anahtar/kaydiricilari durum degiskenlerinden esitler (asagida) */

/* 0.23: kalkan Gizlilik sekmesi, Ayarlar ve Denetim Merkezi'nden ayni yoldan acilir/kapanir */
static void kalkan_ayarla(int ac)
{
	if (!!ac == g_kalkan)
		return;
	g_kalkan = !!ac;
	if (g_kalkan) {
		int fd = open(GZ_KALKAN_BAYRAK, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
		if (fd >= 0)
			close(fd);
		gz_gunluk("gizlilik kalkanı AÇIK");
		gz_kamera_kapat("kalkan: kamera kapatıldı");
		show_toast(LV_SYMBOL_EYE_CLOSE "  Gizlilik kalkanı açık: kamera açılamaz");
	} else {
		unlink(GZ_KALKAN_BAYRAK);
		gz_gunluk("gizlilik kalkanı kapalı");
		show_toast("Gizlilik kalkanı kapatıldı");
	}
	ayar_esitle();
}

static void gz_sw_cb(lv_event_t *e)
{
	kalkan_ayarla(lv_obj_has_state(lv_event_get_target(e), LV_STATE_CHECKED));
}

/* tick_cb her saniye: bayraklari oku, kalkan aciksa uygula */
static void gizlilik_oku(void)
{
	g_kam = dosya_var(GZ_KAMERA_BAYRAK);
	g_mik = dosya_var(GZ_MIK_BAYRAK);
	g_kayit = dosya_var(GZ_KAYIT_BAYRAK);
	if (g_kalkan && (g_kam || g_kayit)) {
		gz_kamera_kapat("kalkan: uzaktan açılan kamera kapatıldı");
		g_kam = dosya_var(GZ_KAMERA_BAYRAK);
		g_kayit = dosya_var(GZ_KAYIT_BAYRAK);
	}
}

static lv_obj_t *gz_satir(lv_obj_t *par, const char *ad)
{
	lv_obj_t *r = kutu(par, LV_FLEX_FLOW_ROW, 8);
	lv_obj_set_flex_align(r, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	lv_obj_t *a = label(r, f_s, ad);
	lv_obj_set_style_text_color(a, C_DIM, 0);
	return label(r, f_s, "-");
}

static void gz_metin(lv_obj_t *l, const char *t)
{
	if (l && strcmp(lv_label_get_text(l), t))
		lv_label_set_text(l, t);
}

/* 0.68: "6%" -> "%6" (Turkce yazim); baska bicim aynen */
static void yuzde_tr(char *o, size_t n, const char *s)
{
	size_t k = strlen(s);
	if (k > 1 && s[k - 1] == '%')
		snprintf(o, n, "%%%.*s", (int)(k - 1), s);
	else
		snprintf(o, n, "%s", s);
}

/* renk degismediyse stil yazma (gereksiz yeniden cizim olmasin) */
static void gz_renk(lv_obj_t *l, lv_color_t c)
{
	if (l && !lv_color_eq(lv_obj_get_style_text_color(l, 0), c))
		lv_obj_set_style_text_color(l, c, 0);
}

static lv_obj_t *kk_ozet, *kk_ara, *kk_har, *kk_zam;	/* 0.46 BUGUNUN KAYITLARI */
static time_t kk_son;

/* 0.53 CANLI GORUNTU (kullanıcı: "kamera goruntuleme bolgesi"). kamera-kayit v7, /tmp/kamera-aktif varken her kareyi
 * /tmp/kamera-son.jpg'ye yazar (1280x720). Bayrak yoksa panel "panel <pid>" icerigiyle yazar ve YALNIZ kendi yazdigini
 * siler. Kare arka planda tjpgd 1/2 ile 640x360 RGB565'e cozulur (cf_coz), tam ekran pencerede 1.4x gosterilir. Goruntu
 * cihazdan CIKMAZ (yalniz bu ekran); kamera gostergesi yanar. Kalkan / ekran koruyucu / bayragin disaridan silinmesi
 * (Merkez "kamera kapat") / 3 dk -> kapanir. Test: PANEL_KAMERA_KARE, PANEL_KAMERA_BAYRAK; {"t":"ui","kamera_goruntu":..}
 * yalniz PANEL_UZAK_TEST=1 iken. */
#define KG_W 640
#define KG_H 360
#define KG_OLCEK 358	/* 256 = 1x; 358 ~ 1.4x -> 896x504 */
#define KG_SURE_MS (3u * 60u * 1000u)
#define KG_GW 896	/* 0.56: kare dogrudan bu kutuya cozulur (LVGL olcekleme yok) */
#define KG_GH 504
static lv_obj_t *dugme(lv_obj_t *par, const char *txt, int32_t w, int32_t h, lv_event_cb_t cb, void *ud, int vurgu);
static lv_obj_t *kg_win, *kg_img, *kg_bilgi;
static uint16_t *kg_tam[3];	/* 0.84: gosterilen (kg_on) / hazir (kg_hazir) / is parcaciginin yazdigi */
static lv_image_dsc_t kg_dsc[3];
static pthread_mutex_t kg_mu = PTHREAD_MUTEX_INITIALIZER;	/* 0.84: kg_on / kg_hazir* */
static int kg_hazir = -1, kg_hazir_cf, kg_hazir_w, kg_hazir_h;
static uint32_t kg_hazir_ms;
static int kg_g2d;	/* 0.85: 0 denenmedi, 1 dogrulandi, -1 kapali (panel omru) */
static int kg_tam_ion;	/* 0.85: kg_tam = vj_hd (ION, G2D hedefi): free edilmez, vj_kapat birakir */
static volatile int kg_g2d_n;	/* 0.85: 5 sn'de G2D ile donusturulen */
static volatile int kg_isci_kaynak;	/* 0.84: is parcaciginin gordugu yeni kare (5 sn) */
static int kg_on, kg_bayrak_bizim, kg_kare_n, kg_ow, kg_oh, kg_log_n;	/* 0.84: kg_sonuc kalkti */
static pthread_t kg_th;
static volatile int kg_is, kg_iptal;
static lv_timer_t *kg_tmr;
static uint32_t kg_t_ac, kg_t_kare, kg_coz_ms;	/* 0.84: kg_t_coz kalkti */
static char kg_hata[128];
static struct timespec kg_son_mt;
static int kg_akici;	/* 0.55 */
static int kg_bekleyen, kg_ist_kaynak, kg_ist_n;	/* 0.56 */
static uint32_t kg_ist_t, kg_ist_coz;
static int kg_ve;	/* 0.81: 0 denenmedi, 1 dogrulandi, -1 kapali (panel omru) */
static volatile int kg_ve_yasak, kg_ve_n;	/* 0.81: wifi-ekran / panel-ve-kapali; 5 sn'de VE ile cozulen */
static uint32_t kg_ve_t;
static int kg_cf[3], kg_cf_yeni;	/* 0.82: tampon bicimi (1 = XRGB8888 VE, 0 = RGB565 yazilim) */
static uint32_t kg_cz_t0, kg_cz_top, kg_cz_n, kg_tik;	/* 0.82: 5 sn cizim suresi / cizim / zamanlayici turu */
static int kg_cz_kayitli;
static uint32_t kg_etiket_t;	/* 0.83: bilgi satiri en son ne zaman yazildi */

static const char *kg_kare_yol(void)
{
	const char *p = getenv("PANEL_KAMERA_KARE");
	return p ? p : "/tmp/kamera-son.jpg";
}

static const char *kg_bayrak_yol(void)
{
	const char *p = getenv("PANEL_KAMERA_BAYRAK");
	return p ? p : GZ_KAMERA_BAYRAK;
}

static void kg_bayrak_yaz(void)
{
	struct stat st;
	kg_bayrak_bizim = 0;
	if (!stat(kg_bayrak_yol(), &st))
		return;	/* Merkez / kayit zaten acmis: dokunma, silme */
	FILE *f = fopen(kg_bayrak_yol(), "w");
	if (f) {
		fprintf(f, "panel %d\n", (int)getpid());
		fclose(f);
		kg_bayrak_bizim = 1;
	}
}

static void kg_bayrak_sil(void)
{
	char b[32] = "";
	FILE *f = fopen(kg_bayrak_yol(), "r");
	if (!f)
		return;
	if (!fgets(b, sizeof b, f))
		b[0] = 0;
	fclose(f);
	if (!strncmp(b, "panel ", 6))
		unlink(kg_bayrak_yol());
}

/* 0.81 (OTONOM 49): kareyi VE ile coz ve 896x504'e sigdir; -1 = yazilim yolu. Ilk kare ayni baytlardan yazilimla da
 * cozulup karsilastirilir (ort. fark > 24 -> VE kapanir). Donanim hatasi -> panel omru boyunca yazilim. */
static int kg_ve_coz(int hedef, int *ow, int *oh)
{
	if (kg_ve < 0)
		return -1;
	if (__atomic_load_n(&kg_ve_yasak, __ATOMIC_ACQUIRE)) {
		if (vj_acik && !kg_tam_ion)	/* 0.85: ION tamponlar ekranda: birakilmaz */
			vj_kapat();
		return -1;
	}
	if (!vj_acik && vj_ac()) {
		kg_ve = -1;
		printf("kamera VE: kullanilamiyor (%s), yazilim cozme\n", vj_hata);
		return -1;
	}
	static uint8_t veri[VJ_BIT];
	int fd = open(kg_kare_yol(), O_RDONLY | O_CLOEXEC);
	if (fd < 0)
		return -1;
	size_t n = 0;
	ssize_t k;
	while (n < sizeof veri && (k = read(fd, veri + n, sizeof veri - n)) > 0)
		n += (size_t)k;
	close(fd);
	if (!n || n >= sizeof veri)
		return -1;
	unsigned sw = 0, sh = 0;
	double t0 = vj_ms();
	int r = vj_coz(veri, n, &sw, &sh);
	if (r == -2) {
		kg_ve = -1;
		if (!kg_tam_ion)	/* 0.85 */
			vj_kapat();
		printf("kamera VE: donanim hatasi (%s), panel acik kaldikca yazilim\n", vj_hata);
		return -1;
	}
	if (r)
		return -1;
	cf_sigdir((int)sw, (int)sh, KG_GW, KG_GH, ow, oh);
	if (!kg_ve) {	/* 0.82: dogrulama RGB565 kopyayla (vj_fark), sonra ekran bicimi */
		if (vj_rgb565(kg_tam[hedef], *ow, *oh, sw, sh))
			return -1;
		FILE *f = fmemopen(veri, n, "rb");
		uint16_t *ref = malloc((size_t)KG_GW * KG_GH * 2);
		int rw = 0, rh = 0, rr = -1;
		char h[64];
		if (f && ref)
			rr = ar_coz_f(f, ref, KG_GW, KG_GH, 1, &rw, &rh, &kg_iptal, h, sizeof h);
		if (f)
			fclose(f);	/* ar_coz_f kapatmaz */
		if (rr || rw != *ow || rh != *oh) {	/* karsilastirilamadi: VE karesi gosterilmez, sonraki karede yeniden */
			free(ref);
			return -1;
		}
		double fark = vj_fark(kg_tam[hedef], ref, (size_t)rw * (size_t)rh);
		free(ref);
		if (fark > 24) {
			kg_ve = -1;
			if (!kg_tam_ion)	/* 0.85 */
				vj_kapat();
			printf("kamera VE: yazilimla fark %.1f (> 24), VE kapatildi\n", fark);
			return -1;
		}
		kg_ve = 1;
		printf("kamera VE: dogrulandi (yazilimla ort. fark %.1f), %ux%u -> %dx%d, %.0f ms\n", fark, sw, sh, *ow, *oh, vj_ms() - t0);
	}
	if (kg_tam_ion && kg_g2d >= 0) {	/* 0.85: G2D, hedef kg_tam[hedef] (= vj_hd[hedef]) */
		if (vj_g2d_cevir(hedef, *ow, *oh, sw, sh)) {
			kg_g2d = -1;
			printf("kamera G2D: %s, CPU donusumu\n", vj_hata);
		} else if (kg_g2d == 0) {	/* ilk kare: CPU donusumuyle karsilastir; tutmazsa diger U/V duzeni */
			size_t np = (size_t)*ow * (size_t)*oh;
			uint32_t *cx = malloc(np * 4);
			double f0 = -1, f1 = -1;
			if (cx && !vj_xrgb8888(cx, *ow, *oh, sw, sh)) {
				f0 = vj_fark_x((const uint32_t *)(void *)kg_tam[hedef], cx, np);
				if (f0 > 8) {
					vj_g2d_uv = 1;
					if (!vj_g2d_cevir(hedef, *ow, *oh, sw, sh))
						f1 = vj_fark_x((const uint32_t *)(void *)kg_tam[hedef], cx, np);
				}
			}
			free(cx);
			if (f0 < 0)
				return -1;	/* bellek yok: sonraki karede yeniden */
			kg_g2d = f0 <= 8 || (f1 >= 0 && f1 <= 8) ? 1 : -1;
			if (kg_g2d < 0)
				vj_g2d_uv = 0;
			printf("kamera G2D: CPU'ya fark %.1f / %.1f -> %s\n", f0, f1,
			       kg_g2d > 0 ? (vj_g2d_uv ? "acik (U1V1U0V0)" : "acik (V1U1V0U0)") : "kapali, CPU donusumu");
		}
		if (kg_g2d > 0) {
			kg_cf_yeni = 1;
			__atomic_add_fetch(&kg_g2d_n, 1, __ATOMIC_RELAXED);
			__atomic_add_fetch(&kg_ve_n, 1, __ATOMIC_RELAXED);
			return 0;
		}
	}
	if (vj_xrgb8888((uint32_t *)(void *)kg_tam[hedef], *ow, *oh, sw, sh))	/* 0.82 */
		return -1;
	kg_cf_yeni = 1;
	__atomic_add_fetch(&kg_ve_n, 1, __ATOMIC_RELAXED);
	return 0;
}

/* 0.81: VE wifi-ekran'in (PC ikinci ekrani H.264/JPEG); o calisirken dokunulmaz. /proc taramasi 2 sn'de bir. */
static int kg_wifi_ekran_var(void)
{
	DIR *d = opendir("/proc");
	struct dirent *e;
	int var = 0;
	while (d && !var && (e = readdir(d))) {
		if (e->d_name[0] < '1' || e->d_name[0] > '9')
			continue;
		char y[64], c[32] = "";
		snprintf(y, sizeof y, "/proc/%.20s/comm", e->d_name);
		FILE *f = fopen(y, "r");
		if (!f)
			continue;
		if (fgets(c, sizeof c, f) && !strncmp(c, "wifi-ekran", 10))
			var = 1;
		fclose(f);
	}
	if (d)
		closedir(d);
	return var;
}

static void *kg_is_parcacigi(void *u)
{
	(void)u;
	setpriority(PRIO_PROCESS, (id_t)syscall(SYS_gettid), 5);	/* 0.55: 15 -> 5 (izlerken akici; Hey Kole yine once) */
	struct timespec son = { 0, 0 };
	while (!kg_iptal) {	/* 0.84: surekli; dosyayi 5 ms'de bir yokla, her yeni kareyi hemen coz */
		struct stat st;
		if (stat(kg_kare_yol(), &st) || (st.st_mtim.tv_sec == son.tv_sec && st.st_mtim.tv_nsec == son.tv_nsec)) {
			usleep(5000);
			continue;
		}
		son = st.st_mtim;
		__atomic_add_fetch(&kg_isci_kaynak, 1, __ATOMIC_RELAXED);
		pthread_mutex_lock(&kg_mu);
		int hedef = 0;
		while (hedef == kg_on || hedef == kg_hazir)	/* uc tampon: biri her zaman bos */
			hedef++;
		pthread_mutex_unlock(&kg_mu);
		uint32_t t0 = tick_ms();
		int w = 0, h = 0;
		kg_cf_yeni = 0;
		int r = kg_ve_coz(hedef, &w, &h);	/* 0.81: once VE */
		if (r) {
			kg_cf_yeni = 0;	/* 0.82: yazilim RGB565 */
			r = ar_coz_yol(kg_kare_yol(), kg_tam[hedef], KG_GW, KG_GH, kg_akici ? 2 : 1, &w, &h, &kg_iptal, kg_hata,
				       sizeof kg_hata);	/* 0.56: 1/2 (net) ya da 1/4 (akici) cozup dogrudan 896x504'e buyut */
		}
		if (r) {
			if (kg_log_n++ < 3)
				printf("kamera goruntusu: kare cozulemedi (%s)\n", kg_hata);
			continue;
		}
		pthread_mutex_lock(&kg_mu);
		kg_hazir = hedef;
		kg_hazir_cf = kg_cf_yeni;
		kg_hazir_w = w;
		kg_hazir_h = h;
		kg_hazir_ms = tick_ms() - t0;
		pthread_mutex_unlock(&kg_mu);
	}
	return NULL;
}

static void kg_kapat(const char *neden)
{
	if (!kg_win && !kg_tmr && !kg_tam[0])
		return;
	if (kg_tmr) {
		lv_timer_delete(kg_tmr);
		kg_tmr = NULL;
	}
	if (kg_is) {
		kg_iptal = 1;
		pthread_join(kg_th, NULL);
		kg_is = 0;
	}
	if (vj_acik && !kg_tam_ion)
		vj_kapat();	/* 0.81: VE yalniz pencere acikken tutulur; 0.85: ION tamponlarla, tamponlar birakildiktan sonra */
	if (kg_bayrak_bizim) {
		kg_bayrak_sil();
		kg_bayrak_bizim = 0;
	}
	lv_obj_t *w = kg_win;
	kg_win = kg_img = kg_bilgi = NULL;
	if (w)
		lv_obj_delete(w);
	for (int i = 0; i < 3; i++) {	/* 0.84: 3 tampon */
		if (kg_tam[i])
			lv_image_cache_drop(&kg_dsc[i]);
		if (!kg_tam_ion)	/* 0.85: ION tamponlarini vj_kapat birakir */
			free(kg_tam[i]);
		kg_tam[i] = NULL;
	}
	if (kg_tam_ion) {
		vj_kapat();
		kg_tam_ion = 0;
	}
	printf("kamera goruntusu: kapandi (%s, %d kare)\n", neden, kg_kare_n);
}

static void kg_sil_cb(lv_event_t *e)	/* pencere disaridan silinirse (ekran yeniden kurulumu) */
{
	if (kg_win && kg_win == lv_event_get_target(e)) {
		kg_win = kg_img = kg_bilgi = NULL;
		kg_kapat("pencere silindi");
	}
}

static void kg_tmr_cb(lv_timer_t *t)
{
	(void)t;
	if (!kg_win)
		return;
	uint32_t now = tick_ms();
	struct stat st;
	if (g_kalkan) {
		kg_kapat("gizlilik kalkani");
		show_toast(LV_SYMBOL_EYE_CLOSE "  Gizlilik kalkanı açık: canlı görüntü kapatıldı");
		return;
	}
	if (ss_obj) {
		kg_kapat("ekran koruyucu");
		return;
	}
	if (now - kg_t_ac > KG_SURE_MS) {
		kg_kapat("3 dk doldu");
		return;
	}
	if (kg_bayrak_bizim && stat(kg_bayrak_yol(), &st)) {
		kg_bayrak_bizim = 0;
		kg_kapat("kamera disaridan kapatildi");
		show_toast(LV_SYMBOL_VIDEO "  Kamera kapatıldı");
		return;
	}
	lv_display_trigger_activity(NULL);	/* izlerken ekran koruyucu gelmesin (en cok 3 dk) */
	kg_tik++;	/* 0.82 */
	if (now - kg_ve_t >= 2000) {	/* 0.81 */
		kg_ve_t = now;
		__atomic_store_n(&kg_ve_yasak, kg_wifi_ekran_var() || dosya_var("/mnt/sd/velinux/panel-ve-kapali"), __ATOMIC_RELEASE);
	}
	pthread_mutex_lock(&kg_mu);	/* 0.84: en son hazir kare */
	int hz = kg_hazir;
	if (hz >= 0) {
		kg_on = hz;
		kg_hazir = -1;
		kg_cf[kg_on] = kg_hazir_cf;
		kg_ow = kg_hazir_w;
		kg_oh = kg_hazir_h;
		kg_coz_ms = kg_hazir_ms;
	}
	pthread_mutex_unlock(&kg_mu);
	{
		if (hz >= 0) {
			uint32_t bp = kg_cf[kg_on] ? 4 : 2;
			lv_image_dsc_t *ds = &kg_dsc[kg_on];
			memset(ds, 0, sizeof *ds);
			ds->header.magic = LV_IMAGE_HEADER_MAGIC;
			ds->header.cf = kg_cf[kg_on] ? LV_COLOR_FORMAT_XRGB8888 : LV_COLOR_FORMAT_RGB565;
			ds->header.w = (uint32_t)kg_ow;
			ds->header.h = (uint32_t)kg_oh;
			ds->header.stride = (uint32_t)kg_ow * bp;
			ds->data_size = (uint32_t)kg_ow * kg_oh * bp;
			ds->data = (const uint8_t *)kg_tam[kg_on];
			lv_image_cache_drop(ds);
			lv_image_set_src(kg_img, ds);
			kg_kare_n++;
			kg_ist_n++;
			kg_ist_coz += kg_coz_ms;
			kg_t_kare = now;
			if (kg_kare_n == 1)
				printf("kamera goruntusu: ilk kare %s, %u ms\n", kg_hata, kg_coz_ms);
		}
	}
	if (now - kg_ist_t >= 5000) {	/* 0.56: 5 sn istatistik */
		printf("kamera istatistik: 5 sn: kaynak %d yeni kare, gosterilen %d, cozme ort %u ms (%s, VE %d, G2D %d) | cizim %u tur ort %u ms,"
		       " zamanlayici %u tur\n", __atomic_exchange_n(&kg_isci_kaynak, 0, __ATOMIC_RELAXED), kg_ist_n,
		       kg_ist_n ? (unsigned)(kg_ist_coz / (uint32_t)kg_ist_n) : 0u, kg_akici ? "akici" : "net",
		       __atomic_exchange_n(&kg_ve_n, 0, __ATOMIC_RELAXED), __atomic_exchange_n(&kg_g2d_n, 0, __ATOMIC_RELAXED),
		       (unsigned)kg_cz_n,
		       kg_cz_n ? (unsigned)(kg_cz_top / kg_cz_n) : 0u, (unsigned)kg_tik);	/* 0.81; 0.82 cizim/tur */
		kg_cz_n = kg_cz_top = kg_tik = 0;
		kg_ist_t = now;
		kg_ist_kaynak = kg_ist_n = 0;
		kg_ist_coz = 0;
	}
	if (now - kg_etiket_t < 1000)	/* 0.83: saniyede bir (her tur ek cizim tetikliyordu) */
		return;
	kg_etiket_t = now;
	uint32_t kalan = (KG_SURE_MS - (now - kg_t_ac)) / 1000;
	if (!kg_kare_n || now - kg_t_kare > 4000)
		lv_label_set_text_fmt(kg_bilgi, "Görüntü bekleniyor…\n%u:%02u sonra kapanır", (unsigned)(kalan / 60), (unsigned)(kalan % 60));
	else
		lv_label_set_text_fmt(kg_bilgi, kg_ve > 0 && vj_acik ? "Canlı · %.1f kare/sn · %u ms · VE\n%u:%02u sonra kapanır"
				      : "Canlı · %.1f kare/sn · %u ms\n%u:%02u sonra kapanır",
				      kg_kare_n * 1000.0 / (double)(now - kg_t_ac + 1), (unsigned)kg_coz_ms,
				      (unsigned)(kalan / 60), (unsigned)(kalan % 60));	/* 0.55: kare/sn */
}

/* 0.82: canli pencere acikken ekran cizim suresi (LVGL render basi -> hazir) */
static void kg_cizim_cb(lv_event_t *e)
{
	if (!kg_win)
		return;
	if (lv_event_get_code(e) == LV_EVENT_RENDER_START) {
		kg_cz_t0 = tick_ms();
	} else if (kg_cz_t0) {
		kg_cz_top += tick_ms() - kg_cz_t0;
		kg_cz_n++;
		kg_cz_t0 = 0;
	}
}

static void kg_kapat_cb(lv_event_t *e)
{
	(void)e;
	kg_kapat("kapat dugmesi");
}

static void kg_akici_cb(lv_event_t *e)	/* 0.55: Akici (1/4) / Net (1/2) */
{
	kg_akici = !kg_akici;
	lv_obj_t *l = lv_obj_get_child(lv_event_get_target(e), 0);
	if (l)
		lv_label_set_text(l, kg_akici ? LV_SYMBOL_IMAGE "  Net görüntü" : LV_SYMBOL_PLAY "  Akıcı görüntü");
	printf("kamera goruntusu: %s\n", kg_akici ? "akici (1/4)" : "net (1/2)");
}

static void av_kapat(const char *neden);	/* 0.56 arsiv (asagida) */
static void kg_ac_cb(lv_event_t *e)
{
	(void)e;
	if (kg_win)
		return;
	av_kapat("canli goruntu acildi");
	if (g_kalkan) {
		show_toast(LV_SYMBOL_EYE_CLOSE "  Gizlilik kalkanı açık: kamera açılamaz");
		return;
	}
	kg_tam_ion = 0;	/* 0.85: once VE + G2D ION tamponlari canli goruntunun kendi tamponlari (ek bellek yok) */
	if (kg_ve >= 0 && kg_g2d >= 0 && !kg_wifi_ekran_var() && !dosya_var("/mnt/sd/velinux/panel-ve-kapali") && !vj_ac() &&
	    !vj_g2d_ac((size_t)KG_GW * KG_GH * 4, 3)) {
		for (int i = 0; i < 3; i++)
			kg_tam[i] = (uint16_t *)(void *)vj_hd[i];
		kg_tam_ion = 1;
	} else {
		if (vj_acik)
			vj_kapat();
		if (kg_g2d >= 0 && kg_ve >= 0)
			printf("kamera G2D: tampon yok (%s), normal bellek + CPU donusumu\n", vj_hata);
		for (int i = 0; i < 3; i++)	/* 0.84: 3 tampon */
			kg_tam[i] = malloc((size_t)KG_GW * KG_GH * 4);	/* 0.56; 0.82: XRGB8888 icin 4 bayt */
	}
	if (!kg_tam[0] || !kg_tam[1] || !kg_tam[2]) {
		for (int i = 0; i < 3; i++) {
			free(kg_tam[i]);
			kg_tam[i] = NULL;
		}
		show_toast(LV_SYMBOL_WARNING "  Bellek yetmedi: canlı görüntü açılamadı");
		return;
	}
	kg_bayrak_yaz();
	kg_win = lv_obj_create(lv_layer_top());
	lv_obj_remove_style_all(kg_win);
	lv_obj_set_size(kg_win, W, H);
	lv_obj_set_style_bg_color(kg_win, lv_color_black(), 0);
	lv_obj_set_style_bg_opa(kg_win, LV_OPA_COVER, 0);
	lv_obj_add_flag(kg_win, LV_OBJ_FLAG_CLICKABLE);	/* alttaki arayuze dokunma gecmesin */
	lv_obj_remove_flag(kg_win, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_add_event_cb(kg_win, kg_sil_cb, LV_EVENT_DELETE, NULL);
	kg_img = lv_image_create(kg_win);
	lv_image_set_antialias(kg_img, false);
	lv_obj_set_size(kg_img, KG_GW, KG_GH);	/* 0.56 */
	lv_image_set_inner_align(kg_img, LV_IMAGE_ALIGN_CENTER);
	lv_obj_set_style_bg_color(kg_img, lv_color_hex(0x161616), 0);
	lv_obj_set_style_bg_opa(kg_img, LV_OPA_COVER, 0);
	lv_obj_align(kg_img, LV_ALIGN_LEFT_MID, 40, 0);
	lv_obj_t *sag = kutu(kg_win, LV_FLEX_FLOW_COLUMN, 18);
	lv_obj_set_width(sag, W - KG_W * KG_OLCEK / 256 - 140);
	lv_obj_align(sag, LV_ALIGN_RIGHT_MID, -40, 0);
	lv_obj_t *bs = label(sag, f_l, LV_SYMBOL_VIDEO "  Canlı görüntü");
	lv_obj_set_style_text_color(bs, lv_color_hex(0xDC2626), 0);
	kg_bilgi = label(sag, f_m, "Görüntü bekleniyor…");
	lv_obj_set_style_text_color(kg_bilgi, lv_color_hex(0xE5E5E5), 0);
	lv_obj_set_style_text_line_space(kg_bilgi, 6, 0);
	lv_obj_t *n = label(sag, f_s, "Bu pencere görüntüyü kaydetmez, dışarı göndermez; yalnız bu ekranda gösterir. "
			       "3 dakika sonra kendiliğinden kapanır.");
	lv_label_set_long_mode(n, LV_LABEL_LONG_WRAP);
	lv_obj_set_width(n, LV_PCT(100));
	lv_obj_set_style_text_color(n, lv_color_hex(0x9CA3AF), 0);
	dugme(sag, kg_akici ? LV_SYMBOL_IMAGE "  Net görüntü" : LV_SYMBOL_PLAY "  Akıcı görüntü", LV_PCT(100), 64, kg_akici_cb, NULL, 0);
	dugme(sag, LV_SYMBOL_CLOSE "  Kapat", LV_PCT(100), 84, kg_kapat_cb, NULL, 1);
	kg_on = kg_kare_n = kg_log_n = 0;
	kg_is = 0;
	kg_son_mt = (struct timespec){0, 0};
	kg_t_ac = tick_ms();
	kg_bekleyen = kg_ist_kaynak = kg_ist_n = 0;
	kg_ist_coz = 0;
	kg_ist_t = tick_ms();
	kg_tmr = lv_timer_create(kg_tmr_cb, 80, NULL);	/* 0.56: 200 -> 80 ms */
	kg_cz_t0 = kg_cz_top = kg_cz_n = kg_tik = 0;	/* 0.82 */
	kg_hazir = -1;	/* 0.84: surekli cozme is parcacigi */
	kg_isci_kaynak = 0;
	kg_iptal = 0;
	kg_is = !pthread_create(&kg_th, NULL, kg_is_parcacigi, NULL);
	kg_etiket_t = 0;	/* 0.83 */
	if (!kg_cz_kayitli) {
		lv_display_add_event_cb(lv_display_get_default(), kg_cizim_cb, LV_EVENT_RENDER_START, NULL);
		lv_display_add_event_cb(lv_display_get_default(), kg_cizim_cb, LV_EVENT_RENDER_READY, NULL);
		kg_cz_kayitli = 1;
	}
	printf("kamera goruntusu: acildi (bayrak %s)\n", kg_bayrak_bizim ? "panel" : "zaten vardi");
}

/* 0.56 KAMERA ARSIVI (kullanıcı: "panel uzerinden eski goruntuleri izleyebileyim fotolari goruntuleyebileyim"). SD SALT OKUMA:
 * kayit parcalari <kamera>/YYYYMMDD/HHMM.mjpg (2 kare/sn, 1 dk), time-lapse <kamera>/zaman/YYYYMMDD.mjpg, fotograflar
 * <kamera>/fotolar + /mnt/sd/fotograf (arsiv_v1.h). Tam ekran: solda tur + liste (en yeni ustte) + bilgi, sagda 896x504
 * goruntu, konum kaydiricisi, Daha eski / Oynat / Daha yeni / Hiz. Kare arka planda bulunup cozulur (ar_kare_bul +
 * ar_coz_parca 1/2). Kayit parcasi bitince daha yeni dakikaya gecer; foto Oynat = 3 sn slayt. */
#define AV_W 896
#define AV_H 504
#define AV_N 200
static const char *kk_kok(void);
static void bg_renk(lv_obj_t *o, lv_color_t c);	/* asagida */
static lv_obj_t *av_win, *av_liste, *av_img, *av_bilgi, *av_sl, *av_oynat_l, *av_hiz_l, *av_tur_b[AR_TUR_N];
static struct ar_oge av_oge[AV_N];
static int av_n, av_tur, av_sec = -1, av_oynuyor, av_on, av_sonuc, av_ow, av_oh, av_kare, av_son, av_istek, av_log_n;
static int av_hiz = 1, av_atla_k;	/* 0.70: 0..4 = 0.5x 1x 2x 4x 8x; is parcacigina atlanacak kare sayisi */
static const char *const av_hiz_ad[5] = {"0.5x", "1x", "2x", "4x", "8x"};
static const uint8_t av_hiz_atla[5] = {0, 0, 0, 1, 3};	/* 4x/8x kare atlayarak (ekrana en cok ~4 kare/sn) */
static uint16_t *av_tam[2];
static lv_image_dsc_t av_dsc[2];
static pthread_t av_th;
static volatile int av_is, av_iptal;
static long av_ofs, av_boy, av_bas, av_uz;
static char av_hata[128], av_yol[400];
static lv_timer_t *av_tmr;
static uint32_t av_t_son, av_coz_ms, av_t_coz;
static const char *const av_tur_ad[AR_TUR_N] = {"Kayıtlar", "Time-lapse", "Fotoğraflar"};

static const char *av_foto_kok(void)
{
	const char *e = getenv("PANEL_FOTO_KOK");
	return e && e[0] ? e : "/mnt/sd/fotograf";
}

static void *av_is_parcacigi(void *u)
{
	(void)u;
	setpriority(PRIO_PROCESS, (id_t)syscall(SYS_gettid), 5);
	int w = 0, h = 0;
	av_son = 0;
	if (av_tur == AR_FOTO) {
		av_sonuc = ar_coz_yol(av_yol, av_tam[1 - av_on], AV_W, AV_H, 0, &w, &h, &av_iptal, av_hata, sizeof av_hata);
	} else {
		long bas = 0, uz = 0;
		int r = -1;
		FILE *f = fopen(av_yol, "rb");
		if (f) {
			r = ar_kare_bul(f, av_ofs, &bas, &uz, 8L * 1024 * 1024);
			for (int k = 0; r == 1 && k < av_atla_k; k++) {	/* 0.70: 4x/8x */
				long b2 = 0, u2 = 0;
				if (ar_kare_bul(f, bas + uz, &b2, &u2, 8L * 1024 * 1024) != 1)
					break;
				bas = b2;
				uz = u2;
			}
			fclose(f);
		}
		if (r == 1) {
			av_bas = bas;
			av_uz = uz;
			av_sonuc = ar_coz_parca(av_yol, bas, uz, av_tam[1 - av_on], AV_W, AV_H, 1, &w, &h, &av_iptal, av_hata, sizeof av_hata);
		} else {
			av_son = 1;
			av_sonuc = -1;
			snprintf(av_hata, sizeof av_hata, "%s", r ? "okuma hatasi" : "dosya sonu");
		}
	}
	av_ow = w;
	av_oh = h;
	__atomic_store_n(&av_is, 2, __ATOMIC_RELEASE);
	return NULL;
}

static void av_bekle(void)	/* calisan cozmeyi durdur */
{
	if (av_is) {
		av_iptal = 1;
		pthread_join(av_th, NULL);
		av_is = 0;
	}
}

static void av_oynat_yaz(void)
{
	if (av_oynat_l)
		lv_label_set_text(av_oynat_l, av_oynuyor ? LV_SYMBOL_PAUSE "  Duraklat" : LV_SYMBOL_PLAY "  Oynat");
	if (av_hiz_l)
		lv_label_set_text_fmt(av_hiz_l, "Hız %s", av_hiz_ad[av_hiz]);	/* 0.70 */
}

static void av_bilgi_yaz(void)
{
	if (!av_bilgi || av_sec < 0)
		return;
	const struct ar_oge *o = &av_oge[av_sec];
	if (av_tur == AR_FOTO) {
		lv_label_set_text_fmt(av_bilgi, "%s\n%d / %d · %u ms", o->ad, av_sec + 1, av_n, (unsigned)av_coz_ms);
	} else if (av_tur == AR_KAYIT) {
		const char *ad = strrchr(o->yol, '/');
		int hh = ad ? (ad[1] - '0') * 10 + (ad[2] - '0') : 0, mm = ad ? (ad[3] - '0') * 10 + (ad[4] - '0') : 0;
		int sn = av_boy > 0 ? (int)(60LL * av_bas / av_boy) : 0;
		lv_label_set_text_fmt(av_bilgi, "%s\n~%02d:%02d:%02d · kare %d · %u ms", o->ad, hh, mm, sn, av_kare, (unsigned)av_coz_ms);
	} else {
		lv_label_set_text_fmt(av_bilgi, "%s\nkare %d · %%%d · %u ms", o->ad, av_kare, av_boy > 0 ? (int)(100LL * av_bas / av_boy) : 0,
				      (unsigned)av_coz_ms);
	}
}

static void av_kapat(const char *neden);

static void av_sec_ac(int i)
{
	if (!av_win || i < 0 || i >= av_n)
		return;
	av_bekle();
	av_sec = i;
	snprintf(av_yol, sizeof av_yol, "%s", av_oge[i].yol);
	av_boy = (long)av_oge[i].bayt;
	av_ofs = av_bas = 0;
	av_kare = 0;
	av_istek = 1;
	av_log_n = 0;
	if (av_tur != AR_FOTO)
		av_oynuyor = 1;	/* kayit / time-lapse secilince oynar */
	av_oynat_yaz();
	uint32_t n = lv_obj_get_child_count(av_liste);
	for (uint32_t k = 0; k < n; k++) {
		lv_obj_t *b = lv_obj_get_child(av_liste, (int32_t)k);
		bg_renk(b, (int)k == i ? C_ACC : lv_color_hex(TH.btn));
	}
	lv_obj_t *sb = lv_obj_get_child(av_liste, i);
	if (sb)
		lv_obj_scroll_to_view(sb, LV_ANIM_OFF);
	if (av_sl)
		lv_slider_set_value(av_sl, 0, LV_ANIM_OFF);
	printf("arsiv: sec %s\n", av_oge[i].ad);
}

static void av_oge_cb(lv_event_t *e)
{
	av_sec_ac((int)(uintptr_t)lv_event_get_user_data(e));
}

static void av_tur_sec(int t)
{
	if (!av_win || t < 0 || t >= AR_TUR_N)
		return;
	av_bekle();
	av_tur = t;
	av_sec = -1;
	av_oynuyor = 0;
	av_kare = 0;
	av_n = ar_listele(kk_kok(), av_foto_kok(), t, av_oge, AV_N);
	lv_obj_clean(av_liste);
	for (int i = 0; i < av_n; i++) {
		lv_obj_t *b = lv_button_create(av_liste);
		lv_obj_set_size(b, LV_PCT(100), 56);
		lv_obj_set_style_radius(b, 12, 0);
		lv_obj_set_style_shadow_width(b, 0, 0);
		lv_obj_set_style_bg_color(b, lv_color_hex(TH.btn), 0);
		lv_obj_t *l = label(b, f_m, av_oge[i].ad);
		lv_obj_set_width(l, LV_PCT(100));
		lv_label_set_long_mode(l, LV_LABEL_LONG_DOT);
		lv_obj_set_style_text_color(l, lv_color_hex(TH.text), 0);
		lv_obj_align(l, LV_ALIGN_LEFT_MID, 4, 0);
		lv_obj_add_event_cb(b, av_oge_cb, LV_EVENT_CLICKED, (void *)(uintptr_t)i);
	}
	for (int k = 0; k < AR_TUR_N; k++)
		if (av_tur_b[k])
			bg_renk(av_tur_b[k], k == t ? C_ACC : lv_color_hex(TH.btn));
	lv_image_set_src(av_img, NULL);
	if (av_sl)
		gizle(av_sl, t == AR_FOTO);
	lv_label_set_text(av_bilgi, av_n ? "Soldan bir öğe seçin" : t == AR_FOTO ? "Fotoğraf yok" : t == AR_ZAMAN ? "Time-lapse kaydı yok"
										      : "Kayıt yok");
	av_oynat_yaz();
	printf("arsiv: %s, %d oge\n", av_tur_ad[t], av_n);
}

static void av_tur_cb(lv_event_t *e)
{
	av_tur_sec((int)(uintptr_t)lv_event_get_user_data(e));
}

static void av_oynat_cb(lv_event_t *e)
{
	(void)e;
	if (av_sec < 0 && av_n)
		av_sec_ac(0);
	else
		av_oynuyor = !av_oynuyor;
	av_oynat_yaz();
}

static void av_hiz_cb(lv_event_t *e)
{
	(void)e;
	av_hiz = (av_hiz + 1) % 5;	/* 0.70: 0.5x -> 1x -> 2x -> 4x -> 8x -> 0.5x */
	av_oynat_yaz();
	printf("arsiv: hiz %s\n", av_hiz_ad[av_hiz]);
}

static void av_git_cb(lv_event_t *e)	/* -1: daha yeni (listede yukari), +1: daha eski */
{
	int y = (int)(intptr_t)lv_event_get_user_data(e);
	if (!av_n)
		return;
	int i = av_sec < 0 ? 0 : av_sec + y;
	if (av_tur == AR_FOTO)
		i = (i + av_n) % av_n;
	av_sec_ac(i);
}

static void av_sl_cb(lv_event_t *e)	/* birakinca o konumdan sonraki kare */
{
	if (av_sec < 0 || av_tur == AR_FOTO)
		return;
	int32_t v = lv_slider_get_value(lv_event_get_target(e));
	av_bekle();
	av_ofs = (long)((long long)v * av_boy / 1000);
	av_istek = 1;
}

static void av_kapat_cb(lv_event_t *e)
{
	(void)e;
	av_kapat("kapat dugmesi");
}

static void av_tmr_cb(lv_timer_t *t)
{
	(void)t;
	if (!av_win)
		return;
	uint32_t now = tick_ms();
	if (ss_obj) {
		av_kapat("ekran koruyucu");
		return;
	}
	if (av_oynuyor)
		lv_display_trigger_activity(NULL);
	if (__atomic_load_n(&av_is, __ATOMIC_ACQUIRE) == 2) {
		pthread_join(av_th, NULL);
		av_is = 0;
		av_coz_ms = now - av_t_coz;
		if (av_tur != AR_FOTO && !av_son)
			av_ofs = av_bas + av_uz;
		if (!av_sonuc) {
			av_on = 1 - av_on;
			lv_image_dsc_t *ds = &av_dsc[av_on];
			memset(ds, 0, sizeof *ds);
			ds->header.magic = LV_IMAGE_HEADER_MAGIC;
			ds->header.cf = LV_COLOR_FORMAT_RGB565;
			ds->header.w = (uint32_t)av_ow;
			ds->header.h = (uint32_t)av_oh;
			ds->header.stride = (uint32_t)av_ow * 2;
			ds->data_size = (uint32_t)av_ow * av_oh * 2;
			ds->data = (const uint8_t *)av_tam[av_on];
			lv_image_cache_drop(ds);
			lv_image_set_src(av_img, ds);
			av_kare++;
			av_t_son = now;
			if (av_kare == 1)
				printf("arsiv: ilk kare %s, %u ms\n", av_hata, (unsigned)av_coz_ms);
			if (av_sl && av_tur != AR_FOTO && av_boy > 0)
				lv_slider_set_value(av_sl, (int32_t)(1000LL * av_bas / av_boy), LV_ANIM_OFF);
			av_bilgi_yaz();
		} else if (av_son) {
			printf("arsiv: %s bitti (%d kare)\n", av_oge[av_sec].ad, av_kare);
			if (av_oynuyor && av_tur == AR_KAYIT && av_sec > 0) {
				av_sec_ac(av_sec - 1);	/* bir sonraki (daha yeni) dakika */
				return;
			}
			av_oynuyor = 0;
			av_oynat_yaz();
			lv_label_set_text_fmt(av_bilgi, "%s\nbitti · %d kare", av_oge[av_sec].ad, av_kare);
		} else if (av_log_n++ < 3) {
			printf("arsiv: kare cozulemedi (%s)\n", av_hata);
		}
	}
	if (!av_is && av_sec >= 0) {
		uint32_t taban = av_tur == AR_KAYIT ? 500u : 200u;	/* 0.70: 1x kare araligi */
		uint32_t aralik = av_tur == AR_FOTO ? 6000u >> av_hiz : taban * 2u * (1u + av_hiz_atla[av_hiz]) >> av_hiz;
		if (!av_istek && av_oynuyor && av_tur == AR_FOTO && now - av_t_son >= aralik) {
			av_sec_ac(av_sec > 0 ? av_sec - 1 : av_n - 1);	/* slayt: daha yeni, en yenide en eskiye don */
			return;
		}
		if (av_istek || (av_oynuyor && av_tur != AR_FOTO && now - av_t_son >= aralik)) {
			av_atla_k = av_istek ? 0 : av_hiz_atla[av_hiz];	/* 0.70: konum secilince tam o kare */
			av_istek = 0;
			av_iptal = 0;
			av_is = 1;
			av_t_coz = now;
			if (pthread_create(&av_th, NULL, av_is_parcacigi, NULL))
				av_is = 0;
		}
	}
}

static void av_kapat(const char *neden)
{
	if (!av_win && !av_tmr && !av_tam[0])
		return;
	if (av_tmr) {
		lv_timer_delete(av_tmr);
		av_tmr = NULL;
	}
	av_bekle();
	lv_obj_t *w = av_win;
	av_win = av_liste = av_img = av_bilgi = av_sl = av_oynat_l = av_hiz_l = NULL;
	for (int k = 0; k < AR_TUR_N; k++)
		av_tur_b[k] = NULL;
	if (w)
		lv_obj_delete(w);
	for (int i = 0; i < 2; i++) {
		if (av_tam[i])
			lv_image_cache_drop(&av_dsc[i]);
		free(av_tam[i]);
		av_tam[i] = NULL;
	}
	av_sec = -1;
	printf("arsiv: kapandi (%s)\n", neden);
}

static void av_sil_cb(lv_event_t *e)
{
	if (av_win && av_win == lv_event_get_target(e)) {
		av_win = NULL;
		av_kapat("pencere silindi");
	}
}

static lv_obj_t *av_dugme(lv_obj_t *par, const char *t, int32_t w, lv_event_cb_t cb, void *ud, lv_obj_t **lab)
{
	lv_obj_t *b = dugme(par, t, w, 48, cb, ud, 0);
	if (lab)
		*lab = lv_obj_get_child(b, 0);
	return b;
}

static void av_ac(int tur)
{
	if (av_win) {
		av_tur_sec(tur);
		return;
	}
	kg_kapat("arsiv acildi");
	for (int i = 0; i < 2; i++)
		av_tam[i] = malloc((size_t)AV_W * AV_H * 2);
	if (!av_tam[0] || !av_tam[1]) {
		for (int i = 0; i < 2; i++) {
			free(av_tam[i]);
			av_tam[i] = NULL;
		}
		show_toast(LV_SYMBOL_WARNING "  Bellek yetmedi: arşiv açılamadı");
		return;
	}
	av_win = lv_obj_create(lv_layer_top());
	lv_obj_remove_style_all(av_win);
	lv_obj_set_size(av_win, W, H);
	lv_obj_set_style_bg_color(av_win, lv_color_hex(0x0B0B0F), 0);
	lv_obj_set_style_bg_opa(av_win, LV_OPA_COVER, 0);
	lv_obj_add_flag(av_win, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_remove_flag(av_win, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_add_event_cb(av_win, av_sil_cb, LV_EVENT_DELETE, NULL);
	lv_obj_t *sol = kutu(av_win, LV_FLEX_FLOW_COLUMN, 10);
	lv_obj_set_size(sol, W - AV_W - 48, H - 24);
	lv_obj_set_pos(sol, 16, 12);
	lv_obj_t *ust = kutu(sol, LV_FLEX_FLOW_ROW, 10);
	lv_obj_set_flex_align(ust, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	lv_obj_t *bs = label(ust, f_l, LV_SYMBOL_VIDEO "  Kamera arşivi");
	lv_obj_set_style_text_color(bs, lv_color_hex(0xF4F4F5), 0);
	dugme(ust, LV_SYMBOL_CLOSE "  Kapat", 160, 52, av_kapat_cb, NULL, 1);
	lv_obj_t *tr = kutu(sol, LV_FLEX_FLOW_ROW, 8);
	for (int k = 0; k < AR_TUR_N; k++)
		av_tur_b[k] = dugme(tr, av_tur_ad[k], 200, 52, av_tur_cb, (void *)(uintptr_t)k, 0);
	av_liste = lv_obj_create(sol);
	lv_obj_remove_style_all(av_liste);
	lv_obj_set_width(av_liste, LV_PCT(100));
	lv_obj_set_flex_grow(av_liste, 1);
	lv_obj_set_flex_flow(av_liste, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_style_pad_row(av_liste, 6, 0);
	lv_obj_set_scroll_dir(av_liste, LV_DIR_VER);
	lv_obj_set_scrollbar_mode(av_liste, LV_SCROLLBAR_MODE_ON);	/* 0.70: kalin, hep gorunur */
	lv_obj_set_style_width(av_liste, 10, LV_PART_SCROLLBAR);
	lv_obj_set_style_radius(av_liste, 5, LV_PART_SCROLLBAR);
	lv_obj_set_style_bg_color(av_liste, C_ACC, LV_PART_SCROLLBAR);
	lv_obj_set_style_bg_opa(av_liste, LV_OPA_70, LV_PART_SCROLLBAR);
	lv_obj_set_style_pad_right(av_liste, 16, 0);	/* cubuk dugmelerin ustune binmesin */
	av_bilgi = label(sol, f_m, "");
	lv_obj_set_width(av_bilgi, LV_PCT(100));
	lv_label_set_long_mode(av_bilgi, LV_LABEL_LONG_WRAP);
	lv_obj_set_style_text_color(av_bilgi, lv_color_hex(0xD4D4D8), 0);
	av_img = lv_image_create(av_win);
	lv_obj_set_size(av_img, AV_W, AV_H);
	lv_image_set_inner_align(av_img, LV_IMAGE_ALIGN_CENTER);
	lv_obj_set_style_bg_color(av_img, lv_color_hex(0x161616), 0);
	lv_obj_set_style_bg_opa(av_img, LV_OPA_COVER, 0);
	lv_obj_set_pos(av_img, W - AV_W - 16, 12);
	/* 0.70: konum cubugu buyuk ve ALTINDA DUGME YOK (kullanıcı: "kaydirirken bazen alttaki daha eski daha yeniye
	 * tiklayabiliyorum"); Daha eski / Oynat / Daha yeni / Hiz sol sutunda, bilgi satirinin altinda */
	av_sl = lv_slider_create(av_win);
	lv_obj_set_size(av_sl, AV_W - 60, 30);
	lv_obj_set_pos(av_sl, W - AV_W + 14, 12 + AV_H + 34);
	lv_slider_set_range(av_sl, 0, 1000);
	lv_obj_set_style_pad_all(av_sl, 11, LV_PART_KNOB);
	lv_obj_set_style_bg_color(av_sl, lv_color_hex(0x3F3F46), LV_PART_MAIN);
	lv_obj_set_ext_click_area(av_sl, 18);
	lv_obj_add_event_cb(av_sl, av_sl_cb, LV_EVENT_RELEASED, NULL);
	lv_obj_t *kt = kutu(sol, LV_FLEX_FLOW_ROW, 8);
	lv_obj_set_flex_align(kt, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	lv_obj_t *kd[4];
	kd[0] = av_dugme(kt, LV_SYMBOL_LEFT " Daha eski", 172, av_git_cb, (void *)(intptr_t)1, NULL);
	kd[1] = av_dugme(kt, LV_SYMBOL_PLAY " Oynat", 150, av_oynat_cb, NULL, &av_oynat_l);
	kd[2] = av_dugme(kt, "Daha yeni " LV_SYMBOL_RIGHT, 172, av_git_cb, (void *)(intptr_t)-1, NULL);
	kd[3] = av_dugme(kt, "Hız 1x", 130, av_hiz_cb, NULL, &av_hiz_l);
	for (int k = 0; k < 4; k++) {
		lv_obj_set_style_pad_hor(kd[k], 4, 0);
		lv_obj_set_style_text_font(lv_obj_get_child(kd[k], 0), f_m, 0);
	}
	av_on = av_kare = 0;
	av_sec = -1;
	av_tmr = lv_timer_create(av_tmr_cb, 60, NULL);
	printf("arsiv: acildi\n");
	av_tur_sec(tur);
}

static void av_ac_cb(lv_event_t *e)
{
	av_ac((int)(uintptr_t)lv_event_get_user_data(e));
}

static void gz_build(lv_obj_t *t)
{
	lv_obj_set_flex_flow(t, LV_FLEX_FLOW_ROW);
	lv_obj_set_style_pad_column(t, 14, 0);
	lv_obj_remove_flag(t, LV_OBJ_FLAG_SCROLLABLE);

	lv_obj_t *c1 = card_i(t, LV_SYMBOL_VIDEO, "KAMERA");
	lv_obj_set_flex_grow(c1, 5);
	lv_obj_set_style_pad_row(c1, 6, 0);
	gz_kam_l = label(c1, f_l, "-");
	gz_kam_alt = label(c1, f_s, "");
	lv_obj_set_width(gz_kam_alt, LV_PCT(100));	/* 0.23: uzun durum metni kart disina tasmasin */
	lv_label_set_long_mode(gz_kam_alt, LV_LABEL_LONG_DOT);
	lv_obj_set_height(gz_kam_alt, lv_font_get_line_height(f_s) + 6);	/* +6: alt bosluk */
	lv_obj_set_style_text_color(gz_kam_alt, C_DIM, 0);
	lv_obj_set_style_pad_bottom(gz_kam_alt, 6, 0);
	gz_v_canli = gz_satir(c1, "Canlı izleme");
	gz_v_kayit = gz_satir(c1, "SD kaydı");
	gz_v_fps = gz_satir(c1, "Yakalama hızı");
	gz_v_parca = gz_satir(c1, "Kayıt parçası");
	gz_v_toplam = gz_satir(c1, "Toplam kayıt");
	gz_v_bos = gz_satir(c1, "SD boş alan");
	gz_v_son = gz_satir(c1, "Son parça");
	gz_v_hata = gz_satir(c1, "Durum");

	/* 0.46: orta sutun = MIKROFON (kisa) + BUGUNUN KAYITLARI (SD salt okuma) */
	lv_obj_t *orta = kutu(t, LV_FLEX_FLOW_COLUMN, 14);
	lv_obj_set_size(orta, 0, LV_PCT(100));
	lv_obj_set_flex_grow(orta, 5);
	lv_obj_t *c2 = card_i(orta, LV_SYMBOL_AUDIO, "MİKROFON");
	lv_obj_set_size(c2, LV_PCT(100), LV_SIZE_CONTENT);
	lv_obj_set_flex_grow(c2, 0);
	gz_mik_l = label(c2, f_l, "-");
	lv_obj_t *mi = label(c2, f_s, "Yalnız KÖLE Merkez açar; panel gösterir.");
	lv_label_set_long_mode(mi, LV_LABEL_LONG_DOT);
	lv_obj_set_width(mi, LV_PCT(100));
	lv_obj_set_style_text_color(mi, C_DIM, 0);
	lv_obj_t *c4 = card_i(orta, LV_SYMBOL_LIST, "BUGÜNÜN KAYITLARI");
	lv_obj_set_size(c4, LV_PCT(100), 0);
	lv_obj_set_flex_grow(c4, 1);
	lv_obj_set_style_pad_row(c4, 6, 0);
	kk_ozet = label(c4, f_m, "-");
	kk_ara = label(c4, f_s, "");
	kk_har = label(c4, f_s, "");
	kk_zam = label(c4, f_s, "");
	lv_obj_t *kl[3] = {kk_ara, kk_har, kk_zam};
	for (int i = 0; i < 3; i++) {
		lv_label_set_long_mode(kl[i], LV_LABEL_LONG_WRAP);
		lv_obj_set_width(kl[i], LV_PCT(100));
		lv_obj_set_style_text_line_space(kl[i], 2, 0);
	}
	lv_obj_set_style_text_color(kk_har, lv_color_hex(TH.acc2), 0);
	lv_obj_set_style_text_color(kk_zam, C_DIM, 0);
	kk_son = 0;	/* yeniden kurulumda hemen doldur */
	dugme(c4, LV_SYMBOL_PLAY "  Kayıtları ve fotoğrafları izle", LV_PCT(100), 60, av_ac_cb, (void *)(uintptr_t)AR_KAYIT, 0);	/* 0.56 */

	lv_obj_t *c3 = card_i(t, LV_SYMBOL_EYE_CLOSE, "GİZLİLİK");
	lv_obj_set_flex_grow(c3, 5);
	gz_btn = lv_button_create(c3);
	lv_obj_set_size(gz_btn, LV_PCT(100), 84);
	lv_obj_set_style_radius(gz_btn, 16, 0);
	lv_obj_set_style_bg_color(gz_btn, lv_color_hex(0xDC2626), 0);
	lv_obj_set_style_bg_color(gz_btn, lv_color_hex(TH.btn), LV_STATE_DISABLED);
	lv_obj_set_style_text_color(gz_btn, lv_color_white(), 0);
	lv_obj_set_style_text_color(gz_btn, C_DIM, LV_STATE_DISABLED);
	lv_obj_add_event_cb(gz_btn, gz_btn_cb, LV_EVENT_CLICKED, NULL);
	gz_btn_l = label(gz_btn, f_mb, LV_SYMBOL_POWER "  Kamerayı kapat");
	lv_obj_center(gz_btn_l);
	dugme(c3, LV_SYMBOL_EYE_OPEN "  Canlı görüntü", LV_PCT(100), 64, kg_ac_cb, NULL, 0);	/* 0.53 */
	lv_obj_t *r = kutu(c3, LV_FLEX_FLOW_ROW, 8);
	lv_obj_set_flex_align(r, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	lv_obj_set_style_pad_top(r, 10, 0);
	label(r, f_mb, "Gizlilik kalkanı");
	gz_sw = lv_switch_create(r);
	lv_obj_set_size(gz_sw, 96, 48);
	if (g_kalkan)
		lv_obj_add_state(gz_sw, LV_STATE_CHECKED);
	lv_obj_add_event_cb(gz_sw, gz_sw_cb, LV_EVENT_VALUE_CHANGED, NULL);
	lv_obj_t *ki = label(c3, f_s, "Açıkken kamera her saniye kapatılır, uzaktan açılamaz. Panel yeniden başlayınca kalkan kapanır.");
	lv_label_set_long_mode(ki, LV_LABEL_LONG_WRAP);
	lv_obj_set_width(ki, LV_PCT(100));
	lv_obj_set_style_text_color(ki, C_DIM, 0);
	gz_son = label(c3, f_s, "");
	lv_label_set_long_mode(gz_son, LV_LABEL_LONG_WRAP);
	lv_obj_set_width(gz_son, LV_PCT(100));
	lv_obj_set_style_pad_top(gz_son, 8, 0);
}

/* 0.46: Gizlilik gorunurken 30 sn'de bir (ilk giriste hemen); SD kamera kayitlari SALT OKUMA */
static const char *kk_kok(void)
{
	const char *e = getenv("PANEL_KAMERA_KOK");	/* host provasi */
	return e && e[0] ? e : "/mnt/sd/kamera";
}

static void kk_tick(void)
{
	time_t now = time(NULL);
	if (!kk_ozet || (kk_son && now >= kk_son && now - kk_son < 30))
		return;
	kk_son = now;
	struct tm tm;
	localtime_r(&now, &tm);
	char gun[16], a[128], b[256], c[192], z[64];
	strftime(gun, sizeof gun, "%Y%m%d", &tm);
	struct kayit_ozet o;
	uint32_t t0 = tick_ms();
	kayit_ozet_oku(kk_kok(), gun, &o);
	kayit_ozet_metin(&o, a, sizeof a, b, sizeof b, c, sizeof c, z, sizeof z);
	gz_metin(kk_ozet, a);
	gz_metin(kk_ara, b);
	gz_metin(kk_har, c);
	gz_metin(kk_zam, z);
	static int son_aralik = -1;	/* ilk tarama, aralik sayisi degisince ve 200 ms'yi asinca gunluge */
	uint32_t ms = tick_ms() - t0;
	if (o.aralik_top != son_aralik || ms > 200)
		printf("kayit ozeti: %s %d parca, %d aralik, %d hareket, %u ms\n", gun, o.parca, o.aralik_top, o.hareket_n, ms);
	son_aralik = o.aralik_top;
}

/* sekme aciksa saniyede bir */
static void gz_tick(void)
{
	if (!gz_kam_l)
		return;
	kk_tick();	/* 0.46 */
	char data[1024], b1[48], b2[48];
	double yas = -1;
	struct stat st;
	int n = 0;
	FILE *f = fopen(GZ_DURUM, "r");
	if (f) {
		n = (int)fread(data, 1, sizeof data - 1, f);
		if (!fstat(fileno(f), &st))
			yas = difftime(time(NULL), st.st_mtime);
		fclose(f);
	}
	data[n > 0 ? n : 0] = 0;
	struct kamera_durum k = kamera_durum_parse(n > 0 ? data : NULL, yas);
	int hizmet = k.gecerli && k.taze;

	if (g_kam && g_kayit)
		gz_metin(gz_kam_l, LV_SYMBOL_VIDEO "  Canlı + kayıt");
	else if (g_kam)
		gz_metin(gz_kam_l, LV_SYMBOL_VIDEO "  Kamera açık");
	else if (g_kayit)
		gz_metin(gz_kam_l, LV_SYMBOL_VIDEO "  Kayıt açık");
	else
		gz_metin(gz_kam_l, LV_SYMBOL_OK "  Kamera kapalı");
	gz_renk(gz_kam_l, g_kam || g_kayit ? C_BAD : C_OK);
	gz_metin(gz_kam_alt, !k.gecerli ? "kamera-kayit hizmeti çalışmıyor (durum dosyası yok)" :
			     hizmet ? "kamera-kayit hizmeti çalışıyor" : "kamera-kayit durumu eski (hizmet durmuş olabilir)");
	gz_metin(gz_v_canli, g_kam ? "AÇIK" : "kapalı");
	gz_metin(gz_v_kayit, g_kayit ? "AÇIK" : "kapalı");
	gz_renk(gz_v_canli, g_kam ? C_BAD : lv_color_hex(TH.text));
	gz_renk(gz_v_kayit, g_kayit ? C_BAD : lv_color_hex(TH.text));
	if (hizmet && (k.canli || k.kayit))
		snprintf(b1, sizeof b1, "%.1f kare/sn", k.fps);
	else
		snprintf(b1, sizeof b1, "-");
	gz_metin(gz_v_fps, b1);
	if (k.gecerli && k.azami_parca)
		snprintf(b1, sizeof b1, "%d / %d", k.parca_sayi, k.azami_parca);
	else if (k.gecerli)
		snprintf(b1, sizeof b1, "%d", k.parca_sayi);
	else
		snprintf(b1, sizeof b1, "-");
	gz_metin(gz_v_parca, b1);
	kd_boyut(b1, sizeof b1, k.gecerli ? k.toplam_mb : -1);
	gz_metin(gz_v_toplam, b1);
	kd_boyut(b2, sizeof b2, k.gecerli ? k.bos_mb : -1);
	gz_metin(gz_v_bos, b2);
	gz_metin(gz_v_son, k.parca[0] ? k.parca : "-");
	gz_metin(gz_v_hata, !k.gecerli ? "-" : k.hata[0] ? k.hata : "sorun yok");
	gz_renk(gz_v_hata, k.hata[0] ? C_WARN : lv_color_hex(TH.text));

	gz_metin(gz_mik_l, g_mik ? LV_SYMBOL_AUDIO "  Mikrofon AÇIK" : LV_SYMBOL_MUTE "  Mikrofon kapalı");
	gz_renk(gz_mik_l, g_mik ? lv_color_hex(0xEA580C) : C_OK);

	int kapatilabilir = g_kam || g_kayit;
	int etkin = kapatilabilir || !g_kalkan;	/* 0.25: kapaliyken "Kaydi baslat"; kalkan aciksa pasif */
	if (etkin != !lv_obj_has_state(gz_btn, LV_STATE_DISABLED)) {
		if (etkin)
			lv_obj_remove_state(gz_btn, LV_STATE_DISABLED);
		else
			lv_obj_add_state(gz_btn, LV_STATE_DISABLED);
	}
	lv_color_t bc = kapatilabilir ? lv_color_hex(0xDC2626) : lv_color_hex(0x15803D);
	if (!lv_color_eq(lv_obj_get_style_bg_color(gz_btn, 0), bc))
		lv_obj_set_style_bg_color(gz_btn, bc, 0);
	gz_metin(gz_btn_l, kapatilabilir ? LV_SYMBOL_POWER "  Kamerayı kapat"
			  : g_kalkan ? LV_SYMBOL_EYE_CLOSE "  Kalkan açık" : LV_SYMBOL_PLAY "  Kaydı başlat");
	char s[140];
	snprintf(s, sizeof s, "Son işlem: %s", gz_son_islem);
	gz_metin(gz_son, s);
}

/* ------------------------------------------------------- 0.29 Hey Kole: konusan yuz */
/* kole-asistan (cihazda uyandirma dinleyicisi) olaylari: {"seq":n,"tur":"cevap|ogren|ogrendi|hata|bilgi","metin":".."}.
 * Yuz ust katmanda; PC tam ekraninda da gorunur (dogrudan cizim ust katman gorununce kendiliginden birakilir). */
#define KY_OLAY "/tmp/asistan-olay.json"
#define KY_DURUM "/tmp/asistan-durum.json"
#define KY_KOMUT "/tmp/asistan-komut"
#define KY_KAPALI "/data/asistan/kapali"
#define KY_ALTYAZI_KAPALI "/data/asistan/altyazi-kapali"
#define KY_ADIM_MS 70
static lv_obj_t *ky_obj, *ky_yazi, *ky_ilerleme, *ky_soru;	/* 0.50: ky_soru anlasilan soru */
static lv_timer_t *ky_tmr;
static char ky_metin[480];
static int ky_n, ky_i, ky_agiz, ky_hedef, ky_ogren, ky_seq_hazir, ky_altyazi = 1, ky_asistan = 1;
static lv_obj_t *ky_sw_asistan, *dm_hk;	/* 0.75: Ayarlar anahtari, Denetim Merkezi dugmesi */
static void ky_asistan_esitle(void);
static uint32_t ky_bitis, ky_kirp_t, ky_kirp_sonra, ky_durum_t, ky_acilis_t;	/* 0.33 acilis */
static long ky_son_seq = -1;
static int ky_tut, ky_dusunuyor;	/* 0.31: dinliyor/dusunuyor -> yuz acik kalir */
static int ky_ses;	/* 0.49: altyazi kole-gemini v14 sesine bagli */
static uint32_t ky_ses_bas, ky_ses_son;
#define KY_TTS_SURE "/tmp/tts-sure"
#define KY_TTS_CALIYOR "/tmp/tts-caliyor"
static const char *ky_durum_metin = "";
/* 0.30: yuz secenekleri (/data/asistan/yuz.conf) */
#define KY_YUZ_CONF "/data/asistan/yuz.conf"
enum { KY_AGIZ_N = 5, KY_GOZ_N = 4, KY_BAKIS_N = 3, KY_RENK_N = 7, KY_KAR_N = 3 };
static int ky_agiz_tur = 1, ky_goz_tur, ky_bakis = 1, ky_renk, ky_karakter;
static float ky_px, ky_py, ky_hx, ky_hy;	/* bebek konumu ve hedefi (merakli bakis) */
static uint32_t ky_bakis_t;
static const char *const ky_agiz_ad[KY_AGIZ_N] = {"Klasik", "Ekolayzır", "Gülümseme", "Izgara", "Dalga"};
static const char *const ky_goz_ad[KY_GOZ_N] = {"Robot", "Yuvarlak", "Vizör", "Sevimli"};
static const char *const ky_bakis_ad[KY_BAKIS_N] = {"Sakin", "Meraklı", "Neşeli"};
static const char *const ky_kar_ad[KY_KAR_N] = {"Sadık köle", "Resmî uşak", "Neşeli robot"};
static const char *const ky_onizle_metin[KY_KAR_N] = {
	"Buyurun efendim, emrinizdeyim. Sizin sadık kölenizim.",
	"Buyurunuz efendim, hizmetinizdeyim. Emirlerinizi bekliyorum.",
	"Bip bop! Kölenizi çağırdınız mı efendim? Hemen buradayım!"};
static const uint32_t ky_renk_govde[KY_RENK_N] = {0, 0x22D3EE, 0x4ADE80, 0xFB923C, 0xC084FC, 0xFF3B3B, 0xF1F5F9};
static const uint32_t ky_renk_bebek[KY_RENK_N] = {0, 0x05262E, 0x052E16, 0x3B1A06, 0x2E1065, 0x380505, 0x1E293B};

static void ky_yuz_yukle(void)
{
	FILE *f = fopen(KY_YUZ_CONF, "r");
	if (!f)
		return;
	char a[32];
	int v;
	while (fscanf(f, " %31[^=]=%d", a, &v) == 2) {
		if (!strcmp(a, "agiz") && v >= 0 && v < KY_AGIZ_N) ky_agiz_tur = v;
		else if (!strcmp(a, "goz") && v >= 0 && v < KY_GOZ_N) ky_goz_tur = v;
		else if (!strcmp(a, "bakis") && v >= 0 && v < KY_BAKIS_N) ky_bakis = v;
		else if (!strcmp(a, "renk") && v >= 0 && v < KY_RENK_N) ky_renk = v;
		else if (!strcmp(a, "karakter") && v >= 0 && v < KY_KAR_N) ky_karakter = v;
	}
	fclose(f);
}

static void ky_yuz_kaydet(void)
{
	mkdir("/data/asistan", 0700);
	FILE *f = fopen(KY_YUZ_CONF ".part", "w");
	if (!f)
		return;
	fprintf(f, "agiz=%d\ngoz=%d\nbakis=%d\nrenk=%d\nkarakter=%d\n", ky_agiz_tur, ky_goz_tur, ky_bakis, ky_renk, ky_karakter);
	if (fclose(f) == 0)
		rename(KY_YUZ_CONF ".part", KY_YUZ_CONF);
	sync();
}
static time_t ky_olay_mtime;
static off_t ky_olay_boy;

static void ky_temizle(void)
{
	if (ky_tmr) {
		lv_timer_delete(ky_tmr);
		ky_tmr = NULL;
	}
	ky_obj = ky_yazi = ky_ilerleme = ky_soru = NULL;
}

static void ky_kapat(void)
{
	printf("hey kole: yuz kapandi\n");	/* 0.49 olcum */
	lv_obj_t *o = ky_obj;
	ky_temizle();
	if (o)
		lv_obj_delete_async(o);
}

static void ky_dokun_cb(lv_event_t *e)
{
	(void)e;
	if (!ky_ogren)	/* ogrenirken yanlislikla kapanmasin */
		ky_kapat();
}

/* yuz alani (gozler + agiz); her adimda yalniz bu alan yeniden cizilir */
static void ky_alan(lv_area_t *a)
{
	lv_area_t o;
	lv_obj_get_coords(ky_obj, &o);
	int32_t cx = (o.x1 + o.x2) / 2;
	a->x1 = cx - 420;
	a->x2 = cx + 420;
	a->y1 = o.y1 + 80;
	a->y2 = o.y1 + 430;
}

static void ky_cizgi(lv_layer_t *l, int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t w, lv_color_t c)
{
	lv_draw_line_dsc_t d;
	lv_draw_line_dsc_init(&d);
	d.p1.x = x1;
	d.p1.y = y1;
	d.p2.x = x2;
	d.p2.y = y2;
	d.color = c;
	d.width = w;
	d.opa = LV_OPA_COVER;
	d.round_start = d.round_end = 1;
	lv_draw_line(l, &d);
}

/* 0.30: secilebilir goz/agiz/renk. ky_agiz 0..~73 (aciklik), ky_px/py bebek kaymasi */
static void ky_ciz_cb(lv_event_t *e)
{
	lv_layer_t *l = lv_event_get_layer(e);
	rg_renk_t rk = rg_renk_sec(0);
	if (ky_renk > 0)
		rk.govde = ky_renk_govde[ky_renk], rk.bebek = ky_renk_bebek[ky_renk];
	lv_color_t G = lv_color_hex(rk.govde), B = lv_color_hex(rk.bebek);
	lv_area_t o;
	lv_obj_get_coords(ky_obj, &o);
	uint32_t t = tick_ms();
	int konusuyor = ky_i < ky_n;
	int32_t cx = (o.x1 + o.x2) / 2, ey = o.y1 + 200;
	if (ky_bakis == 2 && konusuyor)	/* neseli: konusurken gozler ziplar */
		ey += (int32_t)(8 * sinf(t * 0.012f));
	int kirp = (uint32_t)(t - ky_kirp_t) < 150;
	int32_t px = (int32_t)ky_px, py = (int32_t)ky_py;
	if (ky_goz_tur == 2) {	/* vizor: tek bant + sagdan sola kayan isik */
		rg_dortgen(l, cx - 380, ey - 38, cx + 380, ey + 38, 38, B, LV_OPA_COVER);
		rg_dortgen(l, cx - 380, ey - 38, cx + 380, ey + 38, 38, G, 70);
		float f = (t % 2400) / 1200.0f;
		int32_t nx = cx - 330 + (int32_t)(660 * (f < 1 ? f : 2 - f));
		int32_t nr = konusuyor ? 26 + ky_agiz / 6 : 26;
		rg_dortgen(l, nx - nr - 18, ey - nr + 8, nx + nr + 18, ey + nr - 8, nr, G, 110);
		rg_dortgen(l, nx - nr, ey - nr + 6, nx + nr, ey + nr - 6, nr, G, LV_OPA_COVER);
	} else {
		for (int s2 = -1; s2 <= 1; s2 += 2) {
			int32_t gx = cx + s2 * 250;
			if (ky_goz_tur == 3) {	/* sevimli: ^ ^ */
				rg_yay(l, gx, ey + 40, 80, 18, 200, 340, G);
				continue;
			}
			int yuvarlak = ky_goz_tur == 1;
			int32_t yw = yuvarlak ? 82 : 112, yh = kirp ? 8 : yuvarlak ? 82 : 78;
			uint32_t ge = t - ky_acilis_t;	/* 0.33 acilis: 8 px'ten tam boya */
			if (ge < 250 && !kirp)
				yh = 8 + (yh - 8) * (int32_t)ge / 250;
			kirp = kirp || (ge < 250 && yh < 30);	/* acilirken bebek cizilmez */
			rg_dortgen(l, gx - yw, ey - yh, gx + yw, ey + yh, kirp ? 8 : yuvarlak ? LV_RADIUS_CIRCLE : 58, G, LV_OPA_COVER);
			if (!kirp) {
				int32_t bx = gx + px, by = ey + 6 + py;
				rg_dortgen(l, bx - 36, by - 36, bx + 36, by + 36, LV_RADIUS_CIRCLE, B, LV_OPA_COVER);
				rg_dortgen(l, bx + 6, by - 26, bx + 22, by - 10, LV_RADIUS_CIRCLE, lv_color_white(), 235);
			}
		}
	}
	int32_t my = o.y1 + 360, a = ky_agiz;
	switch (ky_agiz_tur) {
	case 1: {	/* ekolayzir: 9 cubuk, ortadakiler yuksek */
		static const int pay[9] = {35, 55, 75, 90, 100, 90, 75, 55, 35};
		for (int i = 0; i < 9; i++) {
			int32_t x = cx - 4 * 34 + i * 34;
			int32_t h = 6 + (int32_t)(a * pay[i] / 100) * (60 + (int32_t)((t / 70 * 2654435761u >> (i + 3)) % 41)) / 100;
			rg_dortgen(l, x - 11, my - h, x + 11, my + h, 6, G, LV_OPA_COVER);
		}
		break;
	}
	case 2:	/* gulumseme: susarken yay, konusurken oval agiz + dil */
		if (a < 8) {
			rg_yay(l, cx, my - 70, 110, 14, 30, 150, G);
		} else {
			int32_t w2 = 60 + a, h2 = 14 + a * 13 / 10;
			rg_dortgen(l, cx - w2, my - h2 / 2, cx + w2, my + h2 / 2, LV_RADIUS_CIRCLE, G, LV_OPA_COVER);
			rg_dortgen(l, cx - w2 + 12, my - h2 / 2 + 10, cx + w2 - 12, my + h2 / 2 - 10, LV_RADIUS_CIRCLE, B, LV_OPA_COVER);
			if (h2 > 50)
				rg_dortgen(l, cx - w2 / 2, my + h2 / 2 - 26, cx + w2 / 2, my + h2 / 2 - 12, 8, lv_color_hex(0xF472B6), LV_OPA_COVER);
		}
		break;
	case 3: {	/* robot izgara: dikdortgen + dis cizgileri */
		int32_t h2 = 22 + a;
		rg_dortgen(l, cx - 175, my - h2 / 2, cx + 175, my + h2 / 2, 12, G, LV_OPA_COVER);
		if (h2 > 30) {
			rg_dortgen(l, cx - 165, my - h2 / 2 + 9, cx + 165, my + h2 / 2 - 9, 6, B, LV_OPA_COVER);
			for (int32_t x = cx - 140; x <= cx + 140; x += 35)
				rg_dortgen(l, x - 4, my - h2 / 2 + 9, x + 4, my + h2 / 2 - 9, 0, G, LV_OPA_COVER);
			rg_dortgen(l, cx - 165, my - 3, cx + 165, my + 3, 0, G, LV_OPA_COVER);
		}
		break;
	}
	case 4: {	/* dalga: genligi sese gore akan sinus */
		float gen = 4 + a * 0.6f, faz = t * 0.012f;
		int32_t ox = cx - 260, oy = my + (int32_t)(gen * sinf(faz));
		for (int32_t x = cx - 250; x <= cx + 260; x += 10) {
			float zarf = 1.0f - fabsf((float)(x - cx) / 270.0f);
			int32_t y = my + (int32_t)(gen * zarf * sinf((x - cx) * 0.045f + faz));
			ky_cizgi(l, ox, oy, x, y, 8, G);
			ox = x;
			oy = y;
		}
		break;
	}
	default: {	/* klasik */
		int32_t yw = 170, yh = 12 + a;
		rg_dortgen(l, cx - yw, my - yh / 2, cx + yw, my + yh / 2, yh / 2, G, LV_OPA_COVER);
		if (yh > 30)
			rg_dortgen(l, cx - yw + 16, my - yh / 2 + 10, cx + yw - 16, my + yh / 2 - 10, (yh - 20) / 2, B, LV_OPA_COVER);
		break;
	}
	}
}

/* 0.30: merakli bakis: 1.5..3.5 sn'de bir yeni hedef, yumusak gecis */
static void ky_bakis_guncelle(uint32_t now)
{
	if (ky_dusunuyor) {	/* 0.31: dusunurken sag yukari */
		ky_hx = 24;
		ky_hy = -20;
	} else if (ky_bakis != 1) {
		ky_hx = ky_hy = 0;
	} else if ((int32_t)(now - ky_bakis_t) > 0) {
		ky_bakis_t = now + 1500 + (uint32_t)(rand() % 2000);
		ky_hx = (float)(rand() % 61 - 30);
		ky_hy = (float)(rand() % 31 - 15);
	}
	ky_px += (ky_hx - ky_px) * 0.25f;
	ky_py += (ky_hy - ky_py) * 0.25f;
}

/* sonraki UTF-8 karakterin bayt sayisi ve agiz hedefi (unlu genis, unsuz orta, bosluk/noktalama kapali) */
static int ky_harf(const char *p, int *hedef)
{
	static const char *const unlu[] = {"a", "e", "ı", "i", "o", "ö", "u", "ü", "A", "E", "I", "İ", "O", "Ö", "U", "Ü"};
	unsigned char c = (unsigned char)p[0];
	int n = c < 0x80 ? 1 : c >> 5 == 6 ? 2 : c >> 4 == 14 ? 3 : 4;
	for (size_t i = 0; i < sizeof unlu / sizeof *unlu; i++)
		if (!strncmp(p, unlu[i], strlen(unlu[i]))) {
			*hedef = 52 + rand() % 22;
			return (int)strlen(unlu[i]);
		}
	*hedef = (c == ' ' || c == '.' || c == ',' || c == '!' || c == '?') ? 0 : 20 + rand() % 14;
	return n;
}

static void ky_ilerleme_yaz(void)
{
	if (!ky_ilerleme)
		return;
	char d[700] = "";
	FILE *f = fopen(KY_DURUM, "r");
	if (f) {
		size_t n = fread(d, 1, sizeof d - 1, f);
		d[n] = 0;
		fclose(f);
	}
	cJSON *j = cJSON_Parse(d);
	cJSON *mod = j ? cJSON_GetObjectItem(j, "mod") : NULL, *on = j ? cJSON_GetObjectItem(j, "ogren_n") : NULL,
	      *ot = j ? cJSON_GetObjectItem(j, "ogren_toplam") : NULL;
	int ogrende = cJSON_IsString(mod) && !strcmp(mod->valuestring, "ogren");
	if (ogrende && cJSON_IsNumber(on) && cJSON_IsNumber(ot))
		lv_label_set_text_fmt(ky_ilerleme, LV_SYMBOL_AUDIO "  Sesiniz öğreniliyor: %d / %d", on->valueint, ot->valueint);
	else
		lv_label_set_text(ky_ilerleme, ky_durum_metin);	/* 0.31 */
	if (!ogrende && j)
		ky_ogren = 0;	/* ogrenme bitti/iptal: son konusmadan sonra kapanir */
	cJSON_Delete(j);
}

static void ky_tmr_cb(lv_timer_t *t)
{
	(void)t;
	if (!ky_obj)
		return;
	uint32_t now = tick_ms();
	if (ky_i < ky_n && !(ky_ses && (int32_t)(now - ky_ses_bas) < 0)) {	/* 0.49: on sessizlik bitmeden yazma */
		ky_i += ky_harf(ky_metin + ky_i, &ky_hedef);
		if (ky_i > ky_n)
			ky_i = ky_n;
		if (ky_yazi && ky_altyazi) {
			char c = ky_metin[ky_i];
			ky_metin[ky_i] = 0;
			lv_label_set_text(ky_yazi, ky_metin);
			ky_metin[ky_i] = c;
		}
		if (ky_i >= ky_n)
			ky_bitis = now + (ky_tut ? 40000 : 2500);	/* 0.31: dinlerken/dusunurken acik kal */
	} else {
		ky_hedef = 0;
	}
	ky_agiz = (ky_agiz + 2 * ky_hedef) / 3;
	ky_bakis_guncelle(now);	/* 0.30 */
	if ((int32_t)(now - ky_kirp_sonra) > 0) {	/* goz kirpma: 2.5..5.5 sn'de bir */
		ky_kirp_t = now;
		ky_kirp_sonra = now + 2500 + (uint32_t)(rand() % 3000);
	}
	if ((uint32_t)(now - ky_durum_t) > 500) {
		ky_durum_t = now;
		ky_ilerleme_yaz();
	}
	lv_area_t a;
	ky_alan(&a);
	lv_obj_invalidate_area(ky_obj, &a);
	lv_display_trigger_activity(disp);	/* konusurken koruyucu acilmasin */
	if (ky_ses && ky_i >= ky_n && (int32_t)(now - ky_ses_son) < 0 && dosya_var(KY_TTS_CALIYOR))
		ky_bitis = now + 600;	/* 0.49: ses bitene kadar acik, sonra 0.6 sn */
	if (ky_i >= ky_n && !ky_ogren && (int32_t)(now - ky_bitis) > 0)
		ky_kapat();
}

/* 0.49: "cevap" olayinda ses suresi varsa altyazi hizi ve baslangici sese baglanir */
static void ky_ses_ayarla(int cevap)
{
	ky_ses = 0;
	if (ky_tmr)
		lv_timer_set_period(ky_tmr, KY_ADIM_MS);
	struct stat st;
	if (!cevap || !ky_tmr || !dosya_var(KY_TTS_CALIYOR) || stat(KY_TTS_SURE, &st) || time(NULL) - st.st_mtime > 10)
		return;
	char b[64];
	int on = 0, sure = 0;
	if (dosya_oku(KY_TTS_SURE, b, sizeof b) <= 0 || sscanf(b, "on=%d sure=%d", &on, &sure) != 2 || sure <= 0 || on < 0 ||
	    on > 5000)
		return;
	int harf = 0;
	for (const unsigned char *p = (const unsigned char *)ky_metin; *p; p++)
		harf += (*p & 0xC0) != 0x80;
	if (harf < 1)
		return;
	int adim = sure / harf;
	adim = adim < 25 ? 25 : adim > 220 ? 220 : adim;
	lv_timer_set_period(ky_tmr, (uint32_t)adim);
	ky_ses = 1;
	ky_ses_bas = tick_ms() + (uint32_t)on;
	ky_ses_son = ky_ses_bas + (uint32_t)sure + 15000;
	printf("hey kole ses: adim %d ms, on %d ms, sure %d ms, %d harf\n", adim, on, sure, harf);
}

static void ky_konus(const char *metin, int ogren)
{
	if (ss_obj)
		ss_hide();
	dm_git(0, 0);
	lv_display_trigger_activity(disp);
	if (!ky_obj) {
		ky_obj = lv_obj_create(lv_layer_top());
		lv_obj_remove_style_all(ky_obj);
		lv_obj_set_size(ky_obj, W, H);
		lv_obj_set_style_bg_color(ky_obj, lv_color_black(), 0);
		lv_obj_set_style_bg_opa(ky_obj, LV_OPA_COVER, 0);
		lv_obj_add_flag(ky_obj, LV_OBJ_FLAG_CLICKABLE);
		lv_obj_remove_flag(ky_obj, LV_OBJ_FLAG_SCROLLABLE);
		lv_obj_add_event_cb(ky_obj, ky_ciz_cb, LV_EVENT_DRAW_MAIN_END, NULL);
		lv_obj_add_event_cb(ky_obj, ky_dokun_cb, LV_EVENT_CLICKED, NULL);
		ky_yazi = label(ky_obj, f_l, "");
		lv_obj_set_width(ky_yazi, W - 240);
		lv_label_set_long_mode(ky_yazi, LV_LABEL_LONG_WRAP);
		lv_obj_set_style_text_align(ky_yazi, LV_TEXT_ALIGN_CENTER, 0);
		lv_obj_set_style_text_color(ky_yazi, lv_color_white(), 0);
		lv_obj_align(ky_yazi, LV_ALIGN_BOTTOM_MID, 0, -26);
		ky_ilerleme = label(ky_obj, f_m, "");
		lv_obj_set_style_text_color(ky_ilerleme, lv_color_hex(0x94A3B8), 0);
		lv_obj_align(ky_ilerleme, LV_ALIGN_TOP_MID, 0, 22);
		ky_soru = label(ky_obj, f_m, "");	/* 0.50: tek satir, uzunsa ... */
		lv_obj_set_size(ky_soru, W - 240, lv_font_get_line_height(f_m));
		lv_label_set_long_mode(ky_soru, LV_LABEL_LONG_DOT);
		lv_obj_set_style_text_align(ky_soru, LV_TEXT_ALIGN_CENTER, 0);
		lv_obj_set_style_text_color(ky_soru, lv_color_hex(0x94A3B8), 0);
		lv_obj_align(ky_soru, LV_ALIGN_TOP_MID, 0, 22);
		ky_tmr = lv_timer_create(ky_tmr_cb, KY_ADIM_MS, NULL);
		ky_kirp_sonra = tick_ms() + 1500;
		ky_acilis_t = gc_acik ? tick_ms() : tick_ms() - 1000;	/* 0.33: gozler 250 ms'de acilir */
	}
	ky_ses = 0;	/* 0.49: yeni metin; ky_ses_ayarla gerekirse yeniden baglar */
	if (ky_tmr)
		lv_timer_set_period(ky_tmr, KY_ADIM_MS);
	snprintf(ky_metin, sizeof ky_metin, "%s", metin && *metin ? metin : "Buyurun efendim.");
	ky_n = (int)strlen(ky_metin);
	ky_i = 0;
	ky_ogren = ogren;
	ky_bitis = tick_ms() + 600000;
	lv_label_set_text(ky_yazi, "");
	if (ky_soru)
		lv_label_set_text(ky_soru, "");	/* 0.50 */
	if (ky_altyazi)
		lv_obj_remove_flag(ky_yazi, LV_OBJ_FLAG_HIDDEN);
	else
		lv_obj_add_flag(ky_yazi, LV_OBJ_FLAG_HIDDEN);
	printf("hey kole: %s%s\n", ogren ? "(ogren) " : "", ky_metin);
}

/* 250 ms'de bir: olay dosyasi degistiyse oku (ilk okuma yalniz sirayi alir, eski olayi gostermez) */
static void ky_olay_tick(lv_timer_t *t)
{
	(void)t;
	static int ky_say;	/* 0.75: bayrak disaridan (Telegram /heykole, sesli kapatma) degisebilir: saniyede bir esitle */
	if (++ky_say >= 4) {
		ky_say = 0;
		int a = !dosya_var(KY_KAPALI);
		if (a != ky_asistan) {
			ky_asistan = a;
			printf("hey kole: %s (bayrak disaridan)\n", a ? "ACIK" : "KAPALI");
			ky_asistan_esitle();
		}
	}
	struct stat st;
	if (stat(KY_OLAY, &st)) {
		ky_seq_hazir = 1;
		return;
	}
	if (st.st_mtime == ky_olay_mtime && st.st_size == ky_olay_boy && ky_seq_hazir)
		return;
	ky_olay_mtime = st.st_mtime;
	ky_olay_boy = st.st_size;
	char b[2048];	/* 0.50: 700 -> 2048 (uzun cevap + soru) */
	FILE *f = fopen(KY_OLAY, "r");
	if (!f)
		return;
	size_t n = fread(b, 1, sizeof b - 1, f);
	b[n] = 0;
	fclose(f);
	cJSON *j = cJSON_Parse(b);
	cJSON *sq = j ? cJSON_GetObjectItem(j, "seq") : NULL, *tur = j ? cJSON_GetObjectItem(j, "tur") : NULL,
	      *mt = j ? cJSON_GetObjectItem(j, "metin") : NULL, *sr = j ? cJSON_GetObjectItem(j, "soru") : NULL;
	if (cJSON_IsNumber(sq) && cJSON_IsString(tur) && cJSON_IsString(mt)) {
		long seq = (long)sq->valuedouble;
		if (!ky_seq_hazir)
			ky_son_seq = seq;
		else if (seq != ky_son_seq) {
			ky_son_seq = seq;
			const char *tr = tur->valuestring;
			if (ky_asistan || !strcmp(tr, "ogren")) {
				ky_konus(mt->valuestring, !strcmp(tr, "ogren"));
				ky_tut = !strcmp(tr, "dinliyor") || !strcmp(tr, "dusunuyor");	/* 0.31 */
				ky_dusunuyor = !strcmp(tr, "dusunuyor");
				ky_ses_ayarla(!strcmp(tr, "cevap"));	/* 0.49 */
				if (ky_soru && ky_altyazi && !strcmp(tr, "cevap") && cJSON_IsString(sr) && sr->valuestring[0]) {	/* 0.50 */
					lv_label_set_text_fmt(ky_soru, "Anladığım: \"%s\"", sr->valuestring);
					printf("hey kole soru: %d bayt\n", (int)strlen(sr->valuestring));
				}
				ky_durum_metin = !strcmp(tr, "dinliyor") ? LV_SYMBOL_AUDIO "  Dinliyorum..."
						 : ky_dusunuyor ? LV_SYMBOL_REFRESH "  Düşünüyorum..." : "";
				ky_ilerleme_yaz();
			}
		}
	}
	ky_seq_hazir = 1;
	cJSON_Delete(j);
}

static void ky_bayrak(const char *yol, int var_olsun)
{
	if (var_olsun) {
		mkdir("/data/asistan", 0700);
		int fd = open(yol, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
		if (fd >= 0)
			close(fd);
	} else {
		unlink(yol);
	}
	sync();
}

static void sw_ayarla(lv_obj_t *sw, int on);	/* asagida */

/* 0.75: Hey Kole durumu gostergelere (Ayarlar anahtari, Denetim Merkezi dugmesi) */
static void ky_asistan_esitle(void)
{
	sw_ayarla(ky_sw_asistan, ky_asistan);
	if (!dm_hk)
		return;
	bg_renk(dm_hk, ky_asistan ? C_ACC : lv_color_hex(TH.btn));
	lv_obj_t *c = lv_obj_get_child(dm_hk, 0);
	if (c) {
		gz_metin(c, ky_asistan ? LV_SYMBOL_AUDIO "  Hey Köle" : LV_SYMBOL_MUTE "  Hey Köle kapalı");
		gz_renk(c, ky_asistan ? lv_color_white() : lv_color_hex(TH.text));
	}
}

/* 0.75: tek yol (Ayarlar, Denetim Merkezi, sesli kapatma) */
static void ky_asistan_ayarla(int ac, const char *kim)
{
	ky_asistan = !!ac;
	ky_bayrak(KY_KAPALI, !ky_asistan);
	ky_asistan_esitle();
	printf("hey kole: %s (%s)\n", ky_asistan ? "ACIK" : "KAPALI", kim);
	show_toast(ky_asistan ? LV_SYMBOL_AUDIO "  Hey Köle dinliyor" : LV_SYMBOL_MUTE "  Hey Köle kapalı (mikrofon bırakıldı)");
}

static void ky_asistan_cb(lv_event_t *e)
{
	ky_asistan_ayarla(lv_obj_has_state(lv_event_get_target(e), LV_STATE_CHECKED), "ayarlar");
}

static void dm_hk_cb(lv_event_t *e)	/* 0.75 Denetim Merkezi */
{
	(void)e;
	ky_asistan_ayarla(!ky_asistan, "denetim merkezi");
}

static void ky_altyazi_cb(lv_event_t *e)
{
	ky_altyazi = lv_obj_has_state(lv_event_get_target(e), LV_STATE_CHECKED);
	ky_bayrak(KY_ALTYAZI_KAPALI, !ky_altyazi);
	if (ky_soru && !ky_altyazi)
		lv_label_set_text(ky_soru, "");	/* 0.50 */
	if (ky_yazi) {
		if (ky_altyazi)
			lv_obj_remove_flag(ky_yazi, LV_OBJ_FLAG_HIDDEN);
		else
			lv_obj_add_flag(ky_yazi, LV_OBJ_FLAG_HIDDEN);
	}
}

static void ky_ogret_cb(lv_event_t *e)
{
	(void)e;
	if (!ky_asistan) {
		show_toast(LV_SYMBOL_WARNING "  Önce Hey Köle anahtarını açın");
		return;
	}
	FILE *f = fopen(KY_KOMUT, "w");
	if (f) {
		fputs("ogren\n", f);
		fclose(f);
	}
	show_toast(LV_SYMBOL_AUDIO "  Öğrenme başlıyor: 5 kez \"Hey Köle\" deyin");
}

static void gc_anim_cb(void *o, int32_t v)
{
	lv_obj_set_style_translate_x((lv_obj_t *)o, v, 0);
}

static void gc_anim_bitti(lv_anim_t *a)
{
	lv_obj_set_style_translate_x((lv_obj_t *)a->var, 0, 0);
	if (gc_olcum) {
		printf("gecis: %u kare / %u ms (yenileme %llu ms, aktarim %llu ms, g2d %llu/%llu us)\n", gc_kare, tick_ms() - gc_t0,
		       (unsigned long long)(gc_cizim_us / 1000), (unsigned long long)(gc_aktarim_us / 1000), pg_say, pg_say ? pg_us / pg_say : 0ULL);
		gc_olcum = 0;
	}
}

static void sekme_degisti_cb(lv_event_t *e)
{
	(void)e;
	sk_gec(lv_tabview_get_tab_active(tabview), 1);	/* 0.45: gosterge + kademeli giris (0.33 kayma sk_tur=0) */
	if (lv_tabview_get_tab_active(tabview) == GZ_SEKME)
		gz_tick();	/* sekmeye girince 1 sn beklemeden doldur */
}

static void hdr_gizlilik_cb(lv_event_t *e)
{
	(void)e;
	if (tabview)
		lv_tabview_set_active(tabview, GZ_SEKME, LV_ANIM_OFF);	/* 0.23: 10 sayfa uzerinden kayan animasyon pahali */
}

/* ===================================================================== 0.23 */
/* Ortak durum (tick_cb doldurur) */
static char wl_ssid[64] = "-";
static char g_ip[32] = "-";
static int g_dbm = -1, g_pc_ok;
static lv_obj_t *hdr_obj, *l_selam;
static hap_t h_dnd;
static uint32_t son_sekme = UINT32_MAX;

/* Gecikmeli silme: olay geri cagrisinin icinden nesne silmek yerine gizle + bir sonraki tick_cb'de sil.
 * (lv_obj_delete_async tema yeniden kurulumuyla ayni turda cakisirsa silinmis nesneyi ikinci kez silebilirdi.) */
static lv_obj_t *cop[8];
static int cop_n;

static void cope_at(lv_obj_t *o)
{
	if (!o)
		return;
	lv_obj_add_flag(o, LV_OBJ_FLAG_HIDDEN);
	if (cop_n < (int)(sizeof cop / sizeof cop[0]))
		cop[cop_n++] = o;
}

static void cop_bosalt(void)
{
	for (int i = 0; i < cop_n; i++)
		lv_obj_delete(cop[i]);
	cop_n = 0;
}

static void sw_ayarla(lv_obj_t *sw, int on)
{
	if (!sw || !!on == lv_obj_has_state(sw, LV_STATE_CHECKED))
		return;
	if (on)
		lv_obj_add_state(sw, LV_STATE_CHECKED);
	else
		lv_obj_remove_state(sw, LV_STATE_CHECKED);
}

static void gizle(lv_obj_t *o, int gizli)
{
	if (!o || !!gizli == lv_obj_has_flag(o, LV_OBJ_FLAG_HIDDEN))
		return;
	if (gizli)
		lv_obj_add_flag(o, LV_OBJ_FLAG_HIDDEN);
	else
		lv_obj_remove_flag(o, LV_OBJ_FLAG_HIDDEN);
}

static void bg_renk(lv_obj_t *o, lv_color_t c)
{
	if (o && !lv_color_eq(lv_obj_get_style_bg_color(o, 0), c))
		lv_obj_set_style_bg_color(o, c, 0);
}

/* stilsiz, tiklanmaz, kaydirilmaz kutu */
static lv_obj_t *duz(lv_obj_t *par, int32_t w, int32_t h)
{
	lv_obj_t *o = lv_obj_create(par);
	lv_obj_remove_style_all(o);
	lv_obj_set_size(o, w, h);
	lv_obj_remove_flag(o, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
	return o;
}

static lv_obj_t *daire(lv_obj_t *par, int32_t cap, lv_color_t c)
{
	lv_obj_t *o = duz(par, cap, cap);
	lv_obj_set_style_radius(o, LV_RADIUS_CIRCLE, 0);
	lv_obj_set_style_bg_opa(o, LV_OPA_COVER, 0);
	lv_obj_set_style_bg_color(o, c, 0);
	return o;
}

/* govdedeki tum alt nesneleri tiklanmaz yap (ust serit: suruklemeyi seridin kendisi alsin) */
static void tiklanmaz(lv_obj_t *o)
{
	lv_obj_remove_flag(o, LV_OBJ_FLAG_CLICKABLE);
	uint32_t n = lv_obj_get_child_count(o);
	for (uint32_t i = 0; i < n; i++)
		tiklanmaz(lv_obj_get_child(o, (int32_t)i));
}

/* genel dugme: vurgu 0 = tema dugmesi, 1 = vurgu rengi, 2 = kirmizi */
static lv_obj_t *dugme(lv_obj_t *par, const char *txt, int32_t w, int32_t h, lv_event_cb_t cb, void *ud, int vurgu)
{
	lv_obj_t *b = lv_button_create(par);
	lv_obj_set_size(b, w, h);
	lv_obj_set_style_radius(b, 16, 0);
	lv_obj_set_style_shadow_width(b, 0, 0);
	lv_obj_set_style_bg_color(b, vurgu == 1 ? C_ACC : vurgu == 2 ? lv_color_hex(0xDC2626) : lv_color_hex(TH.btn), 0);
	lv_obj_set_style_bg_color(b, vurgu ? lv_color_hex(TH.btn2) : C_ACC, LV_STATE_PRESSED);
	lv_obj_set_style_border_width(b, vurgu ? 0 : 1, 0);
	lv_obj_set_style_border_color(b, lv_color_hex(TH.sep), 0);
	lv_obj_set_style_text_color(b, vurgu ? lv_color_white() : lv_color_hex(TH.text), 0);
	lv_obj_set_style_text_color(b, vurgu ? lv_color_hex(TH.text) : lv_color_white(), LV_STATE_PRESSED);
	lv_obj_set_style_text_color(b, C_DIM, LV_STATE_DISABLED);
	lv_obj_set_style_bg_color(b, lv_color_hex(TH.btn), LV_STATE_DISABLED);
	lv_obj_set_style_pad_hor(b, 10, 0);
	lv_obj_t *l = label(b, f_mb, txt);
	lv_obj_center(l);
	if (cb)
		lv_obj_add_event_cb(b, cb, LV_EVENT_CLICKED, ud);
	return b;
}

/* ayar satiri: solda baslik (+ istege bagli aciklama), sagda denetim */
static lv_obj_t *satir(lv_obj_t *par, const char *baslik)
{
	lv_obj_t *r = kutu(par, LV_FLEX_FLOW_ROW, 12);
	lv_obj_set_flex_align(r, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	label(r, f_mb, baslik);
	return r;
}

static lv_obj_t *anahtar(lv_obj_t *par, int on, lv_event_cb_t cb)
{
	lv_obj_t *sw = lv_switch_create(par);
	lv_obj_set_size(sw, 96, 48);
	if (on)
		lv_obj_add_state(sw, LV_STATE_CHECKED);
	lv_obj_add_event_cb(sw, cb, LV_EVENT_VALUE_CHANGED, NULL);
	return sw;
}

static lv_obj_t *soluk(lv_obj_t *par, const lv_font_t *f, const char *t)
{
	lv_obj_t *l = label(par, f, t);
	lv_obj_set_style_text_color(l, C_DIM, 0);
	lv_label_set_long_mode(l, LV_LABEL_LONG_WRAP);
	lv_obj_set_width(l, LV_PCT(100));
	return l;
}

/* bolum/kart basligi (card_i ile ayni gorunum, kart disinda) */
static void baslik_satiri(lv_obj_t *par, const char *ikon, const char *t)
{
	lv_obj_t *r = kutu(par, LV_FLEX_FLOW_ROW, 10);
	lv_obj_set_flex_align(r, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	lv_obj_t *ic = label(r, f_s, ikon);
	lv_obj_set_style_text_color(ic, C_ACC, 0);
	lv_obj_t *b = label(r, f_s, t);
	lv_obj_set_style_text_color(b, C_DIM, 0);
	lv_obj_set_style_text_letter_space(b, 2, 0);
}

/* kalin kaydirici (Denetim Merkezi ve Ayarlar) */
static lv_obj_t *kaydirici(lv_obj_t *par, int32_t w, int32_t h, int min, int max, int deger)
{
	lv_obj_t *s = lv_slider_create(par);
	lv_obj_set_size(s, w, h);
	lv_obj_set_ext_click_area(s, 20);
	lv_slider_set_range(s, min, max);
	lv_slider_set_value(s, deger, LV_ANIM_OFF);
	lv_obj_set_style_bg_color(s, lv_color_hex(TH.sep), LV_PART_MAIN);
	lv_obj_set_style_bg_opa(s, LV_OPA_COVER, LV_PART_MAIN);
	lv_obj_set_style_radius(s, h / 2, LV_PART_MAIN);
	lv_obj_set_style_radius(s, h / 2, LV_PART_INDICATOR);
	lv_obj_set_style_bg_color(s, C_ACC, LV_PART_INDICATOR);
	lv_obj_set_style_bg_color(s, lv_color_white(), LV_PART_KNOB);
	lv_obj_set_style_border_color(s, C_ACC, LV_PART_KNOB);
	lv_obj_set_style_border_width(s, 3, LV_PART_KNOB);
	lv_obj_set_style_pad_all(s, 6, LV_PART_KNOB);
	lv_obj_set_style_shadow_width(s, 0, LV_PART_KNOB);
	return s;
}

/* ------------------------------------------------ cizilmis kucuk simgeler */
/* montserrat'ta ay / kilit / gunes simgesi yok: birkac ucuz nesneyle cizilir */
typedef struct { lv_obj_t *on[2]; uint8_t tur[2]; lv_obj_t *maske, *yazi; } ikon_t;	/* tur 0 dolgu, 1 yay, 2 kenar */

static ikon_t ikon_ay(lv_obj_t *par, int32_t s, lv_color_t zemin)
{
	ikon_t k = {0};
	lv_obj_t *c = duz(par, s, s);
	k.on[0] = daire(c, s * 8 / 10, C_ACC);
	lv_obj_set_pos(k.on[0], s / 10, s / 10);
	k.maske = daire(c, s * 64 / 100, zemin);
	lv_obj_set_pos(k.maske, s * 38 / 100, 0);
	return k;
}

static ikon_t ikon_kilit(lv_obj_t *par, int32_t s)
{
	ikon_t k = {0};
	lv_obj_t *c = duz(par, s, s);
	lv_obj_t *a = lv_arc_create(c);
	lv_obj_remove_style_all(a);
	lv_obj_set_size(a, s * 6 / 10, s * 6 / 10);
	lv_obj_set_pos(a, s * 2 / 10, s * 2 / 100);
	lv_obj_remove_flag(a, LV_OBJ_FLAG_CLICKABLE);
	lv_arc_set_bg_angles(a, 180, 360);
	lv_arc_set_value(a, 0);
	lv_obj_set_style_arc_width(a, s / 9 > 2 ? s / 9 : 2, LV_PART_MAIN);
	lv_obj_set_style_arc_opa(a, LV_OPA_COVER, LV_PART_MAIN);
	lv_obj_set_style_arc_color(a, C_ACC, LV_PART_MAIN);
	lv_obj_set_style_arc_opa(a, LV_OPA_TRANSP, LV_PART_INDICATOR);
	k.on[0] = a;
	k.tur[0] = 1;
	lv_obj_t *g = duz(c, s * 8 / 10, s * 52 / 100);
	lv_obj_set_pos(g, s / 10, s * 44 / 100);
	lv_obj_set_style_radius(g, s / 8, 0);
	lv_obj_set_style_bg_opa(g, LV_OPA_COVER, 0);
	lv_obj_set_style_bg_color(g, C_ACC, 0);
	k.on[1] = g;
	return k;
}

static ikon_t ikon_gunes(lv_obj_t *par, int32_t s)
{
	ikon_t k = {0};
	lv_obj_t *c = duz(par, s, s);
	lv_obj_t *h = duz(c, s, s);
	lv_obj_set_style_radius(h, LV_RADIUS_CIRCLE, 0);
	lv_obj_set_style_border_width(h, s / 10 > 2 ? s / 10 : 2, 0);
	lv_obj_set_style_border_color(h, C_ACC, 0);
	lv_obj_set_style_border_opa(h, LV_OPA_COVER, 0);
	k.on[0] = h;
	k.tur[0] = 2;
	k.on[1] = daire(c, s * 44 / 100, C_ACC);
	lv_obj_center(k.on[1]);
	return k;
}

static ikon_t ikon_yazi(lv_obj_t *par, const char *sembol)
{
	ikon_t k = {0};
	k.yazi = label(par, f_mb, sembol);
	lv_obj_set_style_text_color(k.yazi, C_ACC, 0);
	return k;
}

static void ikon_renk(ikon_t *k, lv_color_t on, lv_color_t zemin)
{
	for (int i = 0; i < 2; i++) {
		if (!k->on[i])
			continue;
		if (k->tur[i] == 1)
			lv_obj_set_style_arc_color(k->on[i], on, LV_PART_MAIN);
		else if (k->tur[i] == 2)
			lv_obj_set_style_border_color(k->on[i], on, 0);
		else
			lv_obj_set_style_bg_color(k->on[i], on, 0);
	}
	if (k->maske)
		lv_obj_set_style_bg_color(k->maske, zemin, 0);
	if (k->yazi)
		gz_renk(k->yazi, on);
}

/* ---------------------------------------------- Denetim Merkezi kutucuklari */
enum { DK_WIFI, DK_BT, DK_KAMERA, DK_KALKAN, DK_DND, DK_GECE, DK_TEMA, DK_KILIT, DK_KORUYUCU, DK_OTOKOR, DK_N };
typedef struct { lv_obj_t *kap, *baslik, *alt; ikon_t ik; int durum; } kutucuk_t;
static kutucuk_t dk[DK_N];

/* durum: 0 pasif, 1 acik (vurgu), 2 tehlike (kirmizi), 3 koruma (yesil) */
static void kutucuk_durum(kutucuk_t *k, int durum, const char *alt)
{
	if (!k->kap)
		return;
	if (alt)
		gz_metin(k->alt, alt);
	if (k->durum == durum)
		return;
	k->durum = durum;
	lv_color_t zemin = durum == 1 ? C_ACC : durum == 2 ? lv_color_hex(0xDC2626) : durum == 3 ? lv_color_hex(0x15803D)
										     : lv_color_hex(TH.btn);
	lv_obj_set_style_bg_color(k->kap, zemin, 0);
	lv_obj_set_style_border_opa(k->kap, durum ? LV_OPA_TRANSP : LV_OPA_COVER, 0);
	gz_renk(k->baslik, durum ? lv_color_white() : lv_color_hex(TH.text));
	gz_renk(k->alt, durum ? lv_color_hex(0xF1F5F9) : C_DIM);
	ikon_renk(&k->ik, durum ? lv_color_white() : C_ACC, zemin);
}

static void dm_kutucuk_cb(lv_event_t *e);

/* ikon_tur: 0 yazi sembolu, 1 ay, 2 kilit */
static void kutucuk_kur(int id, lv_obj_t *par, int32_t w, int32_t h, int ikon_tur, const char *sembol, const char *baslik)
{
	kutucuk_t *k = &dk[id];
	k->kap = lv_obj_create(par);
	lv_obj_remove_style_all(k->kap);
	lv_obj_set_size(k->kap, w, h);
	lv_obj_set_style_radius(k->kap, 22, 0);
	lv_obj_set_style_bg_opa(k->kap, LV_OPA_COVER, 0);
	lv_obj_set_style_bg_color(k->kap, lv_color_hex(TH.btn), 0);
	lv_obj_set_style_border_width(k->kap, 1, 0);
	lv_obj_set_style_border_color(k->kap, lv_color_hex(TH.sep), 0);
	lv_obj_set_style_border_width(k->kap, 3, LV_STATE_PRESSED);	/* basinca ince vurgu cercevesi (ucuz) */
	lv_obj_set_style_border_color(k->kap, C_ACC, LV_STATE_PRESSED);
	lv_obj_set_style_border_opa(k->kap, LV_OPA_COVER, LV_STATE_PRESSED);
	lv_obj_set_style_pad_all(k->kap, 16, 0);
	lv_obj_set_flex_flow(k->kap, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_style_pad_row(k->kap, 2, 0);
	lv_obj_remove_flag(k->kap, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_add_flag(k->kap, LV_OBJ_FLAG_EVENT_BUBBLE);	/* perdeyi yukari kaydirma kutucuk ustunden de calissin */
	lv_obj_add_event_cb(k->kap, dm_kutucuk_cb, LV_EVENT_CLICKED, (void *)(uintptr_t)id);
	k->ik = ikon_tur == 1 ? ikon_ay(k->kap, 30, lv_color_hex(TH.btn)) : ikon_tur == 2 ? ikon_kilit(k->kap, 30)
	      : ikon_tur == 3 ? ikon_gunes(k->kap, 28) : ikon_yazi(k->kap, sembol);
	lv_obj_t *bos = duz(k->kap, 1, 1);
	lv_obj_set_flex_grow(bos, 1);
	k->baslik = label(k->kap, w < 210 && h <= 144 && id >= DK_DND ? f_m : f_mb, baslik);	/* kucuk: ince yazi */
	if (id >= DK_DND)
		lv_obj_set_style_pad_hor(k->kap, 12, 0);
	k->alt = label(k->kap, f_s, "");
	lv_obj_set_width(k->alt, LV_PCT(100));
	lv_label_set_long_mode(k->alt, LV_LABEL_LONG_DOT);
	lv_obj_set_height(k->alt, lv_font_get_line_height(f_s));
	lv_obj_set_style_text_color(k->alt, C_DIM, 0);
	k->durum = 0;
}

/* ------------------------------------------------------ Denetim Merkezi */
#define DM_H 452		/* gorunen yukseklik */
#define DM_UST 28		/* ust yuvarlak koseler ekran disinda kalsin */
#define DM_TAM (DM_H + DM_UST)
static lv_obj_t *dm_perde, *dm_sayfa, *dm_saat, *dm_tarih, *dm_bld, *dm_sl_br, *dm_l_br, *dm_l_br2, *dm_sl_ses, *dm_l_ses,
	*dm_l_ses2, *dm_ses_ik;
static ikon_t dm_br_ik;
static int dm_acik, dm_surukle, dm_tik_iptal;
static int32_t dm_y = -DM_TAM, dm_bas_y, dm_bas_sayfa_y;
static uint32_t dm_bas_ms, dm_bld_surum = UINT32_MAX;
static lv_obj_t *not_win;	/* not duzenleyici (asagida) */
static void kilit_goster(void);
static void dm_guncelle(void);

static int dm_engel(void)
{
	return pc_full || ss_obj || kilitli || wf_win || not_win || gz_mb;
}

static void dm_konum(int32_t y)
{
	if (y < -DM_TAM)
		y = -DM_TAM;
	if (y > -DM_UST)
		y = -DM_UST;
	if (y == dm_y || !dm_sayfa)
		return;
	dm_y = y;
	lv_obj_set_y(dm_sayfa, y);
}

static void dm_yok_et(void)
{
	cope_at(dm_sayfa);
	cope_at(dm_perde);
	dm_sayfa = dm_perde = dm_saat = dm_tarih = dm_bld = dm_sl_br = dm_l_br = dm_l_br2 = dm_sl_ses = dm_l_ses = dm_l_ses2 = dm_ses_ik = NULL;
	dm_hk = NULL;	/* 0.75 */
	memset(dk, 0, sizeof dk);
	memset(&dm_br_ik, 0, sizeof dm_br_ik);
	dm_y = -DM_TAM;
	dm_acik = dm_surukle = 0;
	dm_bld_surum = UINT32_MAX;
}

static void dm_anim_cb(void *o, int32_t v)
{
	if (o == dm_sayfa)
		dm_konum(v);
}

static void dm_anim_bitti(lv_anim_t *a)
{
	if (a->var != dm_sayfa)
		return;
	printf("denetim merkezi: %s\n", dm_acik ? "acildi" : "kapandi");
	if (!dm_acik)
		dm_yok_et();
}

static void dm_olustur(void);

static void dm_git(int ac, int anim)
{
	if (ac && !dm_sayfa) {
		if (dm_engel())
			return;
		dm_olustur();
	}
	if (!dm_sayfa) {
		dm_acik = 0;
		return;
	}
	dm_acik = ac;
	dm_surukle = 0;
	lv_anim_delete(dm_sayfa, dm_anim_cb);
	int32_t hedef = ac ? -DM_UST : -DM_TAM;
	if (!anim || hedef == dm_y) {
		dm_konum(hedef);
		printf("denetim merkezi: %s\n", ac ? "acildi" : "kapandi");
		if (!ac)
			dm_yok_et();
		return;
	}
	int32_t yol = hedef > dm_y ? hedef - dm_y : dm_y - hedef;
	lv_anim_t a;
	lv_anim_init(&a);
	lv_anim_set_var(&a, dm_sayfa);
	lv_anim_set_exec_cb(&a, dm_anim_cb);
	lv_anim_set_values(&a, dm_y, hedef);
	lv_anim_set_duration(&a, 80 + 140 * (uint32_t)yol / DM_TAM);	/* tam yol 220 ms */
	lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
	lv_anim_set_completed_cb(&a, dm_anim_bitti);
	lv_anim_start(&a);
}

static void dm_perde_cb(lv_event_t *e)
{
	(void)e;
	dm_git(0, 1);
}

static void dm_kapat_cb(lv_event_t *e)
{
	(void)e;
	if (!dm_tik_iptal)
		dm_git(0, 1);
}

static void dm_ayarlar_cb(lv_event_t *e)
{
	(void)e;
	if (dm_tik_iptal)
		return;
	dm_git(0, 0);
	if (tabview)
		lv_tabview_set_active(tabview, S_AYARLAR, LV_ANIM_OFF);
}

/* 0.24: Denetim Merkezi kisayolu: robot goz koruyucuyu hemen ac (ayar Karisik ise karisik, degilse robot goz) */
static void dm_robot_cb(lv_event_t *e)
{
	(void)e;
	if (dm_tik_iptal)
		return;
	dm_git(0, 0);
	if (!ss_obj && !pc_full)
		ss_show_tur(ss_tur == 2 ? 2 : 1);
}

/* ust seritten asagi surukle: perde parmagi izler; birakinca yeterince indiyse acilir. Dokunus = ac/kapa. */
static void dm_hdr_cb(lv_event_t *e)
{
	lv_event_code_t c = lv_event_get_code(e);
	lv_indev_t *in = lv_indev_active();
	lv_point_t p;
	if (!in)
		return;
	lv_indev_get_point(in, &p);
	if (c == LV_EVENT_PRESSED) {
		dm_bas_y = p.y;
		dm_surukle = 0;
		dm_bas_ms = tick_ms();
	} else if (c == LV_EVENT_PRESSING) {
		int32_t dy = p.y - dm_bas_y;
		if (!dm_surukle && dy > 12 && !dm_acik) {
			if (!dm_sayfa) {
				if (dm_engel())
					return;
				dm_olustur();
			}
			dm_acik = 1;
			dm_surukle = 1;
			lv_anim_delete(dm_sayfa, dm_anim_cb);
		}
		if (dm_surukle == 1)
			dm_konum(p.y + 64 - DM_TAM);	/* perdenin alt kenari parmagin biraz altinda */
	} else if (c == LV_EVENT_RELEASED || c == LV_EVENT_PRESS_LOST) {
		int32_t dy = p.y - dm_bas_y;
		if (dm_surukle == 1) {
			int hizli = (uint32_t)(tick_ms() - dm_bas_ms) < 300 && dy > 40;
			dm_surukle = 0;
			dm_git(dm_y + DM_TAM > DM_H / 3 || hizli, 1);
		} else if (c == LV_EVENT_RELEASED && dy < 12 && dy > -12) {
			dm_git(!dm_acik, 1);	/* dokunus yedegi */
		}
	}
}

/* perdeyi yukari kaydirarak kapat (kutucuklardan da kabarir) */
static void dm_sayfa_cb(lv_event_t *e)
{
	lv_event_code_t c = lv_event_get_code(e);
	lv_indev_t *in = lv_indev_active();
	lv_point_t p;
	if (!in || !dm_sayfa)
		return;
	lv_indev_get_point(in, &p);
	if (c == LV_EVENT_PRESSED) {
		dm_bas_y = p.y;
		dm_bas_sayfa_y = dm_y;
		dm_surukle = 0;
		dm_tik_iptal = 0;
		dm_bas_ms = tick_ms();
	} else if (c == LV_EVENT_PRESSING) {
		int32_t dy = p.y - dm_bas_y;
		if (!dm_surukle && dy < -28) {
			dm_surukle = 2;
			dm_tik_iptal = 1;	/* kaydirma bir kutucugun ustunde basladiysa o kutucuk tetiklenmesin */
			lv_anim_delete(dm_sayfa, dm_anim_cb);
		}
		if (dm_surukle == 2)
			dm_konum(dm_bas_sayfa_y + (dy < 0 ? dy : 0));
	} else if (c == LV_EVENT_RELEASED || c == LV_EVENT_PRESS_LOST) {
		if (dm_surukle == 2) {
			int32_t yukari = -DM_UST - dm_y;
			int hizli = (uint32_t)(tick_ms() - dm_bas_ms) < 300;
			dm_surukle = 0;
			dm_git(!(yukari > DM_H / 4 || hizli), 1);
		}
	}
}

static void dnd_ayarla(int on);

/* 0.53 BLUETOOTH BAGLAN / KES. BT yongasi MCU'nun D4 tunelinde; mcu-servis-v6 /tmp/mcu-komut FIFO'su "ID HEX.." satiri
 * alir (yasaklilari kendisi reddeder). Komut ciftleri Melis IVT244'ten (0xE91A08CE: AT#CA + AT#CC; 0xE91A0960: AT#CD +
 * AT#CB); anlamlar OVC3860 belgesinden: CA esleme/gorunur, CC son cihaza baglan, CD baglantiyi kes, CB eslemeden cik.
 * KALICI AYAR DEGIL (kalici harita: UART'tan kalici BT alani yalniz AT#MM adi). Radyoyu tamamen kapatan komut bilinmiyor. */
#define BT_MCU_KOMUT "/tmp/mcu-komut"
static struct bt_medya bt_medya_oku(void);	/* asagida (Denetim Merkezi) */
static int mcu_at_gonder(const char *at)
{
	char sat[128];
	int o = snprintf(sat, sizeof sat, "D4");
	for (const char *p = at; *p && o < (int)sizeof sat - 12; p++)
		o += snprintf(sat + o, sizeof sat - (size_t)o, " %02x", (unsigned char)*p);
	o += snprintf(sat + o, sizeof sat - (size_t)o, " 0d 0a\n");
	const char *yol = getenv("PANEL_MCU_KOMUT") ? getenv("PANEL_MCU_KOMUT") : BT_MCU_KOMUT;
	int fd = open(yol, O_WRONLY | O_NONBLOCK | O_CLOEXEC);
	if (fd < 0)
		return -1;
	ssize_t w = write(fd, sat, (size_t)o);
	close(fd);
	return w == o ? 0 : -1;
}

static void bt_baglanti(int bagla, int sessiz)	/* 0.55: sessiz = toast yok (bekci) */
{
	int r = mcu_at_gonder(bagla ? "AT#CA" : "AT#CD");	/* sira onemli: once CA/CD */
	r |= mcu_at_gonder(bagla ? "AT#CC" : "AT#CB");
	printf("bluetooth %s: %s\n", bagla ? "baglan (AT#CA+AT#CC)" : "kes (AT#CD+AT#CB)", r ? "MCU hizmeti yok" : "gonderildi");
	if (sessiz)
		return;
	if (r)
		show_toast(LV_SYMBOL_WARNING "  MCU hizmeti yanıt vermiyor; Bluetooth komutu gönderilemedi");
	else
		show_toast(bagla ? LV_SYMBOL_BLUETOOTH "  Bluetooth: bağlanma isteği gönderildi (görünür + son telefon)"
				 : LV_SYMBOL_BLUETOOTH "  Bluetooth: bağlantıyı kesme isteği gönderildi");
}

/* 0.55 BLUETOOTH ACIK/KAPALI AYARI: radyoyu kapatan komut yok; "kapali" = baglantiyi kes + gorunmez ol, telefon
 * yeniden baglanirsa 15 sn'de bir tekrar kes, acilista (panel 25 sn) bir kez kes. Ayar /data/bt-kapali. */
static int bt_kapali = -1;	/* -1: dosya henuz okunmadi */
static lv_obj_t *bt_sw;
static uint32_t bt_son_kes;

static int bt_kapali_mi(void)
{
	if (bt_kapali < 0) {
		char y[192];
		veri_yol(y, sizeof y, "bt-kapali");
		bt_kapali = dosya_var(y);
	}
	return bt_kapali;
}

static void bt_ayarla(int kapat)
{
	char y[192];
	veri_yol(y, sizeof y, "bt-kapali");
	bt_kapali = !!kapat;
	if (kapat) {
		int fd = open(y, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
		if (fd >= 0)
			close(fd);
	} else {
		unlink(y);
	}
	sync();
	bt_son_kes = tick_ms();
	printf("bluetooth ayari: %s\n", kapat ? "KAPALI" : "ACIK");
	bt_baglanti(!kapat, 0);
	if (bt_sw)
		sw_ayarla(bt_sw, !kapat);
}

static void bt_sw_cb(lv_event_t *e)
{
	bt_ayarla(!lv_obj_has_state(lv_event_get_target(e), LV_STATE_CHECKED));
}

static void bt_bekci(int bagli)	/* bt_tick'ten saniyede bir */
{
	static uint32_t bas, aralik = 1000;	/* 0.79: yeniden kesme araligi 1-2-4-8-15 sn (denetim v3 B3) */
	static int ilk;
	uint32_t now = tick_ms();
	if (!bas)
		bas = now;
	if (!bagli || !bt_kapali_mi())
		aralik = 1000;	/* baglanti dustu: sonraki ilk kesme yine hizli */
	if (!bt_kapali_mi())
		return;
	if (!ilk && now - bas > 25000) {
		ilk = 1;
		bt_son_kes = now;
		printf("bluetooth kapali: acilista kesiliyor\n");
		bt_baglanti(0, 1);
		return;
	}
	if (bagli && now - bt_son_kes > aralik) {	/* 0.56: 15 -> 5 sn; 0.78: 1 sn; 0.79: "bagli" takilirsa geri cekilir */
		bt_son_kes = now;
		aralik = aralik >= 8000 ? 15000 : aralik * 2;
		printf("bluetooth kapali: telefon baglandi, yeniden kesiliyor (sonraki en erken %u sn)\n", aralik / 1000);
		bt_baglanti(0, 1);
	}
}

/* 0.55 TIME-LAPSE: kamera-kayit v3+ <kamera>/zaman-acik varken her N sn'de bir kareyi gunluk zaman/YYYYMMDD.mjpg'ye
 * ekler (N: zaman-aralik 5..3600, 30 gun saklama, kalkan bastirir). Panel yalniz bayragi ve araligi yazar. */
/* 0.74: 1 sn ve 3 sn (kamera-kayit v9: 1..3600), olay tespiti <kamera>/zaman-olay, doluluk satiri zl_dl */
#define ZL_N 6
static const int zl_aralik_sn[ZL_N] = {1, 3, 10, 30, 60, 300};
static const char *const zl_aralik_ad[ZL_N] = {"1 sn", "3 sn", "10 sn", "30 sn", "1 dk", "5 dk"};
static lv_obj_t *zl_sw, *zl_l, *zl_b[ZL_N], *zl_olay_sw, *zl_dl;

static void zl_yol(char *o, size_t n, const char *ad)
{
	snprintf(o, n, "%s/%s", kk_kok(), ad);
}

static int zl_acik(void)
{
	char y[300];
	zl_yol(y, sizeof y, "zaman-acik");
	return dosya_var(y);
}

static int zl_aralik(void)
{
	char y[300], b[16];
	zl_yol(y, sizeof y, "zaman-aralik");
	if (dosya_oku(y, b, sizeof b) > 0)
		return zaman_aralik_dogrula(atoi(b));	/* 0.74: 1..3600 */
	return 60;
}

static int zl_olay_acik(void)	/* 0.74 */
{
	char y[300];
	zl_yol(y, sizeof y, "zaman-olay");
	return dosya_var(y);
}

static void zl_esitle(void)
{
	int ac = zl_acik(), ar = zl_aralik(), ol = zl_olay_acik();
	if (zl_sw)
		sw_ayarla(zl_sw, ac);
	if (zl_olay_sw)
		sw_ayarla(zl_olay_sw, ol);
	for (int i = 0; i < ZL_N; i++)
		if (zl_b[i]) {
			bg_renk(zl_b[i], zl_aralik_sn[i] == ar ? C_ACC : lv_color_hex(TH.btn));
			lv_obj_t *c = lv_obj_get_child(zl_b[i], 0);
			if (c)
				gz_renk(c, zl_aralik_sn[i] == ar ? lv_color_white() : lv_color_hex(TH.text));
		}
	if (!zl_l)
		return;
	char m[200], y[320], gun[16], js[1024];
	if (zl_dl) {	/* 0.74: doluluk (kamera-kayit v9 durum JSON'u) */
		int n = dosya_oku(GZ_DURUM, js, sizeof js);
		struct zaman_durum zd = zaman_durum_parse(n > 0 ? js : NULL);
		zaman_doluluk_metni(m, sizeof m, &zd);
		gz_metin(zl_dl, m);
		gz_renk(zl_dl, zd.dolu ? C_WARN : C_DIM);
	}
	if (!ac) {
		zaman_durum_metni(m, sizeof m, 0, ar, ol, 0, 0);
		gz_metin(zl_l, m);
		return;
	}
	time_t t = time(NULL);
	struct tm tm;
	localtime_r(&t, &tm);
	strftime(gun, sizeof gun, "%Y%m%d", &tm);
	snprintf(y, sizeof y, "%s/zaman/%s.mjpg", kk_kok(), gun);
	struct stat st;
	double mb = stat(y, &st) ? 0 : st.st_size / 1048576.0;
	zaman_durum_metni(m, sizeof m, 1, ar, ol, mb, g_kalkan);	/* 0.74 */
	gz_metin(zl_l, m);
}

static void zl_ayarla(int ac)
{
	char y[300];
	mkdir(kk_kok(), 0755);
	zl_yol(y, sizeof y, "zaman-acik");
	if (ac) {
		int fd = open(y, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
		if (fd >= 0)
			close(fd);
	} else {
		unlink(y);
	}
	sync();
	printf("time-lapse: %s (her %d sn)\n", ac ? "ACIK" : "KAPALI", zl_aralik());
	show_toast(ac ? (g_kalkan ? LV_SYMBOL_EYE_CLOSE "  Time-lapse açıldı; gizlilik kalkanı kapanınca başlar"
				  : LV_SYMBOL_VIDEO "  Time-lapse başladı (SD kart: kamera/zaman)")
		      : LV_SYMBOL_STOP "  Time-lapse durdu");
	zl_esitle();
}

static void zl_sw_cb(lv_event_t *e)
{
	zl_ayarla(lv_obj_has_state(lv_event_get_target(e), LV_STATE_CHECKED));
}

/* 0.74: araligi yaz (dokunma ve sesli islem) */
static void zl_aralik_yaz(int sn)
{
	char y[300], b[16];
	sn = zaman_aralik_dogrula(sn);
	mkdir(kk_kok(), 0755);
	zl_yol(y, sizeof y, "zaman-aralik");
	int n = snprintf(b, sizeof b, "%d\n", sn);
	int fd = open(y, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
	if (fd >= 0) {
		if (write(fd, b, (size_t)n) != n)
			printf("time-lapse: aralik yazilamadi\n");
		close(fd);
	}
	sync();
	printf("time-lapse araligi: %d sn\n", sn);
	zl_esitle();
}

static void zl_aralik_cb(lv_event_t *e)
{
	uintptr_t i = (uintptr_t)lv_event_get_user_data(e);
	if (i < ZL_N)	/* 0.74: "& 3" yerine */
		zl_aralik_yaz(zl_aralik_sn[i]);
}

/* 0.74: olay tespiti (kamera-kayit v9 time-lapse sirasinda dakikalik hareket puani, ek kare, Telegram fotografi) */
static void zl_olay_ayarla(int ac)
{
	char y[300];
	mkdir(kk_kok(), 0755);
	zl_yol(y, sizeof y, "zaman-olay");
	if (ac) {
		int fd = open(y, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
		if (fd >= 0)
			close(fd);
	} else {
		unlink(y);
	}
	sync();
	gz_gunluk(ac ? "time-lapse olay tespiti açıldı (panel)" : "time-lapse olay tespiti kapatıldı (panel)");
	printf("time-lapse olay tespiti: %s\n", ac ? "ACIK" : "KAPALI");
	show_toast(!ac ? LV_SYMBOL_STOP "  Olay tespiti kapalı"
		   : g_kalkan ? LV_SYMBOL_EYE_CLOSE "  Olay tespiti açık; gizlilik kalkanı kapanınca çalışır"
		   : zl_acik() ? LV_SYMBOL_EYE_OPEN "  Olay tespiti açık: hareket anları ek kare + Telegram"
			       : LV_SYMBOL_EYE_OPEN "  Olay tespiti açık (time-lapse açılınca çalışır)");
	zl_esitle();
}

static void zl_olay_sw_cb(lv_event_t *e)
{
	zl_olay_ayarla(lv_obj_has_state(lv_event_get_target(e), LV_STATE_CHECKED));
}

/* 0.57 Denetim Merkezi Kamera: Kapali -> Kayit -> Time-lapse -> Kapali. Yalniz canli (Merkez/panel izleyicisi) aciksa
 * eskisi gibi onayli kapatma. Kayit -> Time-lapse: yalniz kamera-kayit'in yarattigi BOS kamera-aktif silinir. */
static void kamera_mod_dongu(void)
{
	if (g_kalkan) {
		show_toast(LV_SYMBOL_EYE_CLOSE "  Gizlilik kalkanı açık: önce kalkanı kapatın");
		return;
	}
	if (g_kayit) {
		unlink(GZ_KAYIT_BAYRAK);	/* 0.77: kamera-kayit v10 kendi "kayit" isaretini kendisi siler; BOS bayrak artik
						 * kole-kopru KAMERA 1'in (PC canli), silinmez (inceleme 24.10) */
		g_kayit = 0;
		zl_ayarla(1);
		gz_gunluk("kamera modu: kayıt -> time-lapse (Denetim Merkezi)");
		printf("kamera modu: time-lapse\n");
		show_toast(LV_SYMBOL_VIDEO "  Kayıt durdu, time-lapse başladı · tekrar dokun: kapat");
	} else if (zl_acik()) {
		zl_ayarla(0);
		gz_gunluk("kamera modu: time-lapse -> kapalı (Denetim Merkezi)");
		printf("kamera modu: kapali\n");
		show_toast(LV_SYMBOL_STOP "  Kamera kapalı · tekrar dokun: kayıt");
	} else if (g_kam) {
		gz_btn_cb(NULL);	/* yalniz canli: onayli kapatma */
	} else if (!gz_kayit_baslat()) {
		printf("kamera modu: kayit\n");
		show_toast(LV_SYMBOL_VIDEO "  SD kaydı başladı · tekrar dokun: time-lapse");
	}
}

static void dm_kutucuk_cb(lv_event_t *e)
{
	int id = (int)(uintptr_t)lv_event_get_user_data(e);
	if (dm_tik_iptal)
		return;
	switch (id) {
	case DK_WIFI:
		dm_git(0, 0);
		wf_open_cb(NULL);
		return;
	case DK_BT:	/* 0.55: dokunma = Bluetooth ayarini cevir (Acik/Kapali) */
		bt_ayarla(!bt_kapali_mi());
		break;
	case DK_KAMERA:	/* 0.57: 3 mod dongusu */
		kamera_mod_dongu();
		break;
	case DK_KALKAN:
		kalkan_ayarla(!g_kalkan);
		break;
	case DK_DND:
		dnd_ayarla(!dnd);
		break;
	case DK_GECE:
		br_auto = !br_auto;
		br_save();
		ayar_esitle();
		show_toast(br_auto ? "Gece modu açık: parlaklık otomatik (23-07 kısık)" : "Gece modu kapalı: manuel parlaklık");
		break;
	case DK_TEMA:
		tema_sec((tema + 1) % TEMA_N);	/* arayuz yeniden kurulur, perde acik kalir */
		return;
	case DK_KILIT:
		dm_git(0, 0);
		kilit_goster();
		return;
	case DK_KORUYUCU:
		dm_git(0, 0);
		if (!ss_obj && !pc_full)
			ss_show();
		return;
	case DK_OTOKOR:
		ss_on = !ss_on;
		br_save();
		ayar_esitle();
		break;
	}
	dm_guncelle();
}

static void dm_br_cb(lv_event_t *e)
{
	lv_event_code_t c = lv_event_get_code(e);
	if (!dm_sl_br)
		return;
	if (c == LV_EVENT_VALUE_CHANGED) {
		br_auto = 0;	/* elle surukleme manuele gecer */
		br_manual = lv_slider_get_value(dm_sl_br);
		br_apply();
		lv_label_set_text_fmt(dm_l_br, "%%%d", br_manual);
		gz_metin(dm_l_br2, "Manuel");
	} else if (c == LV_EVENT_RELEASED) {
		br_save();
		ayar_esitle();
	}
}

static void dm_ses_cb(lv_event_t *e)
{
	lv_event_code_t c = lv_event_get_code(e);
	if (!dm_sl_ses)
		return;
	if (c == LV_EVENT_VALUE_CHANGED) {
		ses_duzey = lv_slider_get_value(dm_sl_ses);
		ses_sessiz = 0;
		lv_label_set_text_fmt(dm_l_ses, "%%%d", ses_duzey);
	} else if (c == LV_EVENT_RELEASED) {
		br_save();
		ayar_esitle();
	}
}

static void dm_sessiz_cb(lv_event_t *e)
{
	(void)e;
	if (dm_tik_iptal)
		return;
	ses_sessiz = !ses_sessiz;
	br_save();
	ayar_esitle();
}

static lv_obj_t *dm_kaydirici_kart(lv_obj_t *par, int ses, lv_obj_t **sl, lv_obj_t **deger, lv_obj_t **alt)
{
	lv_obj_t *k = lv_obj_create(par);
	lv_obj_remove_style_all(k);
	lv_obj_set_size(k, 450, 144);
	lv_obj_set_style_radius(k, 22, 0);
	lv_obj_set_style_bg_opa(k, LV_OPA_COVER, 0);
	lv_obj_set_style_bg_color(k, lv_color_hex(TH.btn), 0);
	lv_obj_set_style_border_width(k, 1, 0);
	lv_obj_set_style_border_color(k, lv_color_hex(TH.sep), 0);
	lv_obj_set_style_pad_hor(k, 20, 0);
	lv_obj_set_style_pad_ver(k, 14, 0);
	lv_obj_set_flex_flow(k, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_flex_align(k, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
	lv_obj_remove_flag(k, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_add_flag(k, LV_OBJ_FLAG_EVENT_BUBBLE);
	lv_obj_t *r = kutu(k, LV_FLEX_FLOW_ROW, 12);
	lv_obj_set_flex_align(r, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	lv_obj_remove_flag(r, LV_OBJ_FLAG_CLICKABLE);
	if (ses) {
		dm_ses_ik = lv_button_create(r);	/* hoparlor simgesi = sessiz dugmesi */
		lv_obj_remove_style_all(dm_ses_ik);
		lv_obj_set_size(dm_ses_ik, 40, 34);
		lv_obj_set_ext_click_area(dm_ses_ik, 14);
		lv_obj_add_event_cb(dm_ses_ik, dm_sessiz_cb, LV_EVENT_CLICKED, NULL);
		lv_obj_t *il = label(dm_ses_ik, f_mb, LV_SYMBOL_VOLUME_MAX);
		lv_obj_set_style_text_color(il, C_ACC, 0);
		lv_obj_center(il);
	} else
		dm_br_ik = ikon_gunes(r, 28);
	lv_obj_t *t = label(r, f_mb, ses ? "Ses düzeyi" : "Parlaklık");
	lv_obj_set_flex_grow(t, 1);
	*deger = label(r, f_mb, "");
	*sl = kaydirici(k, LV_PCT(100), 30, ses ? 0 : 5, 100, ses ? ses_duzey : br_manual);
	lv_obj_add_event_cb(*sl, ses ? dm_ses_cb : dm_br_cb, LV_EVENT_VALUE_CHANGED, NULL);
	lv_obj_add_event_cb(*sl, ses ? dm_ses_cb : dm_br_cb, LV_EVENT_RELEASED, NULL);
	*alt = label(k, f_s, "");
	lv_obj_set_width(*alt, LV_PCT(100));
	lv_label_set_long_mode(*alt, LV_LABEL_LONG_DOT);
	lv_obj_set_height(*alt, lv_font_get_line_height(f_s));
	lv_obj_set_style_text_color(*alt, ses ? C_WARN : C_DIM, 0);
	return k;
}

static void dm_olustur(void)
{
	lv_obj_t *scr = lv_screen_active();

	dm_perde = lv_obj_create(scr);	/* perde disi: karartma + dokununca kapat */
	lv_obj_remove_style_all(dm_perde);
	lv_obj_set_size(dm_perde, W, H);
	lv_obj_set_style_bg_opa(dm_perde, LV_OPA_50, 0);
	lv_obj_set_style_bg_color(dm_perde, lv_color_black(), 0);
	lv_obj_remove_flag(dm_perde, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_add_event_cb(dm_perde, dm_perde_cb, LV_EVENT_CLICKED, NULL);

	dm_sayfa = lv_obj_create(scr);	/* ekranin son cocugu: opak alanda alttaki arayuz hic cizilmez */
	lv_obj_remove_style_all(dm_sayfa);
	lv_obj_set_size(dm_sayfa, W, DM_TAM);
	dm_y = -DM_TAM;
	lv_obj_set_pos(dm_sayfa, 0, dm_y);
	lv_obj_set_style_bg_opa(dm_sayfa, LV_OPA_COVER, 0);
	lv_obj_set_style_bg_color(dm_sayfa, C_CARD, 0);
	lv_obj_set_style_radius(dm_sayfa, DM_UST, 0);
	lv_obj_set_style_border_width(dm_sayfa, 1, 0);
	lv_obj_set_style_border_side(dm_sayfa, LV_BORDER_SIDE_BOTTOM, 0);
	lv_obj_set_style_border_color(dm_sayfa, lv_color_hex(TH.sep), 0);
	lv_obj_set_style_text_color(dm_sayfa, lv_color_hex(TH.text), 0);
	lv_obj_set_style_pad_top(dm_sayfa, DM_UST + 14, 0);
	lv_obj_set_style_pad_hor(dm_sayfa, 24, 0);
	lv_obj_set_style_pad_bottom(dm_sayfa, 8, 0);
	lv_obj_set_style_pad_row(dm_sayfa, 14, 0);
	lv_obj_set_flex_flow(dm_sayfa, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_flex_align(dm_sayfa, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	lv_obj_remove_flag(dm_sayfa, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_add_event_cb(dm_sayfa, dm_sayfa_cb, LV_EVENT_PRESSED, NULL);
	lv_obj_add_event_cb(dm_sayfa, dm_sayfa_cb, LV_EVENT_PRESSING, NULL);
	lv_obj_add_event_cb(dm_sayfa, dm_sayfa_cb, LV_EVENT_RELEASED, NULL);
	lv_obj_add_event_cb(dm_sayfa, dm_sayfa_cb, LV_EVENT_PRESS_LOST, NULL);

	/* baslik satiri: saat + tarih | son bildirim | Ayarlar, kapat */
	lv_obj_t *ust = kutu(dm_sayfa, LV_FLEX_FLOW_ROW, 18);
	lv_obj_set_height(ust, 56);
	lv_obj_remove_flag(ust, LV_OBJ_FLAG_CLICKABLE);	/* bosluklardan yukari kaydirma perdeye ulassin */
	lv_obj_set_flex_align(ust, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	dm_saat = label(ust, f_l, "--:--");
	dm_tarih = label(ust, f_m, "");
	lv_obj_set_style_text_color(dm_tarih, C_DIM, 0);
	dm_bld = label(ust, f_s, "");
	lv_obj_set_flex_grow(dm_bld, 1);
	lv_obj_set_width(dm_bld, 0);
	lv_label_set_long_mode(dm_bld, LV_LABEL_LONG_DOT);
	lv_obj_set_height(dm_bld, lv_font_get_line_height(f_s));
	lv_obj_set_style_text_color(dm_bld, C_DIM, 0);
	lv_obj_set_style_text_align(dm_bld, LV_TEXT_ALIGN_RIGHT, 0);
	dm_hk = dugme(ust, LV_SYMBOL_AUDIO "  Hey Köle", 260, 52, dm_hk_cb, NULL, 0);	/* 0.75 ac/kapat */
	ky_asistan_esitle();
	dugme(ust, LV_SYMBOL_EYE_OPEN "  Robot göz", 220, 52, dm_robot_cb, NULL, 0);	/* 0.24 kisayol */
	dugme(ust, LV_SYMBOL_SETTINGS "  Ayarlar", 200, 52, dm_ayarlar_cb, NULL, 0);
	dugme(ust, LV_SYMBOL_UP, 72, 52, dm_kapat_cb, NULL, 0);

	/* ana satir: baglanti kutucuklari | kaydiricilar | hizli anahtarlar */
	lv_obj_t *ana = kutu(dm_sayfa, LV_FLEX_FLOW_ROW, 16);
	lv_obj_set_height(ana, 300);
	lv_obj_remove_flag(ana, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_add_flag(ana, LV_OBJ_FLAG_EVENT_BUBBLE);	/* kutucuk olaylari perdeye kadar kabarsin */
	lv_obj_t *s1 = kutu(ana, LV_FLEX_FLOW_ROW_WRAP, 12);
	lv_obj_remove_flag(s1, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_add_flag(s1, LV_OBJ_FLAG_EVENT_BUBBLE);	/* kutucuk olaylari perdeye kadar kabarsin */
	lv_obj_set_size(s1, 420, 300);
	kutucuk_kur(DK_WIFI, s1, 204, 144, 0, LV_SYMBOL_WIFI, "Wi-Fi");
	kutucuk_kur(DK_BT, s1, 204, 144, 0, LV_SYMBOL_BLUETOOTH, "Bluetooth");
	kutucuk_kur(DK_KAMERA, s1, 204, 144, 0, LV_SYMBOL_VIDEO, "Kamera");
	kutucuk_kur(DK_KALKAN, s1, 204, 144, 0, LV_SYMBOL_EYE_CLOSE, "Kalkan");
	lv_obj_t *s2 = kutu(ana, LV_FLEX_FLOW_COLUMN, 12);
	lv_obj_remove_flag(s2, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_add_flag(s2, LV_OBJ_FLAG_EVENT_BUBBLE);	/* kutucuk olaylari perdeye kadar kabarsin */
	lv_obj_set_size(s2, 450, 300);
	dm_kaydirici_kart(s2, 0, &dm_sl_br, &dm_l_br, &dm_l_br2);
	dm_kaydirici_kart(s2, 1, &dm_sl_ses, &dm_l_ses, &dm_l_ses2);
	lv_obj_t *s3 = kutu(ana, LV_FLEX_FLOW_ROW_WRAP, 12);
	lv_obj_remove_flag(s3, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_add_flag(s3, LV_OBJ_FLAG_EVENT_BUBBLE);	/* kutucuk olaylari perdeye kadar kabarsin */
	lv_obj_set_height(s3, 300);
	lv_obj_set_flex_grow(s3, 1);
	lv_obj_set_width(s3, 0);
	/* 3 x 208 px: "Rahatsiz etme" (ince 28 px = 180 px) tek satira sigar */
	kutucuk_kur(DK_DND, s3, 208, 144, 1, NULL, "Rahatsız etme");
	kutucuk_kur(DK_GECE, s3, 208, 144, 3, NULL, "Gece modu");
	kutucuk_kur(DK_TEMA, s3, 208, 144, 0, LV_SYMBOL_TINT, "Tema");
	kutucuk_kur(DK_KILIT, s3, 208, 144, 2, NULL, "Panel kilidi");
	kutucuk_kur(DK_KORUYUCU, s3, 208, 144, 0, LV_SYMBOL_IMAGE, "Koruyucu");
	kutucuk_kur(DK_OTOKOR, s3, 208, 144, 0, LV_SYMBOL_LOOP, "Oto koruyucu");

	/* tutamac */
	lv_obj_t *tut = duz(dm_sayfa, 84, 6);
	lv_obj_set_style_radius(tut, 3, 0);
	lv_obj_set_style_bg_opa(tut, LV_OPA_COVER, 0);
	lv_obj_set_style_bg_color(tut, C_DIM, 0);
	for (int i = 0; i < DK_N; i++)
		dk[i].durum = -1;	/* ilk guncellemede renkler yazilsin */
	dm_guncelle();
}

static struct bt_medya bt_medya_oku(void)
{
	char data[1024];
	int n = dosya_oku("/tmp/bt-durum.json", data, sizeof data);
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	return bt_medya_coz(n > 0 && n < (int)sizeof data - 1 ? data : "", now.tv_sec + now.tv_nsec / 1e9);
}

static void dm_guncelle(void)
{
	char b[160];
	if (!dm_sayfa)
		return;
	time_t now = time(NULL);
	struct tm tm;
	localtime_r(&now, &tm);
	if (tm.tm_year > 120) {
		snprintf(b, sizeof b, "%02d:%02d", tm.tm_hour, tm.tm_min);
		gz_metin(dm_saat, b);
		snprintf(b, sizeof b, "%d %s %s", tm.tm_mday, aylar[tm.tm_mon], gunler[tm.tm_wday]);
		gz_metin(dm_tarih, b);
	}
	if (dm_bld_surum != bld_surum) {
		dm_bld_surum = bld_surum;
		char s[128];
		if (bildirim_al(0, s, sizeof s))
			snprintf(b, sizeof b, "%s", dnd ? "Rahatsız etme açık · bildirim yok" : "Bildirim yok");
		else
			snprintf(b, sizeof b, LV_SYMBOL_BELL "  %s", s);
		gz_metin(dm_bld, b);
	}
	int bagli = strcmp(g_ip, "-") != 0;
	if (bagli && wifi_duzey(g_dbm))
		snprintf(b, sizeof b, "%s · %d dBm", wl_ssid, g_dbm);
	else
		snprintf(b, sizeof b, "%s", bagli ? wl_ssid : "Bağlı değil · kur");
	kutucuk_durum(&dk[DK_WIFI], bagli, b);
	struct bt_medya m = bt_medya_oku();
	kutucuk_durum(&dk[DK_BT], !bt_kapali_mi() && m.taze && m.baglanti == 1,
		      bt_kapali_mi() ? "Kapalı · dokun: aç" : !m.taze ? "Açık · bilgi yok" : m.baglanti == 1 ? "Açık · bağlı" : "Açık · bağlı değil");	/* 0.55 */
	int zl_on = zl_acik() && !g_kalkan;	/* 0.55: time-lapse da kamera calistirir */
	kutucuk_durum(&dk[DK_KAMERA], g_kam || g_kayit || zl_on ? 2 : 0,
		      g_kam && g_kayit ? "Canlı + kayıt" : g_kayit ? "Kayıt açık" : zl_on ? "Time-lapse açık"
		      : g_kam ? "Canlı · kapat" : "Kapalı");	/* 0.57: dokunma Kapali -> Kayit -> Time-lapse */	/* 0.53: kutucuga sigsin */
	kutucuk_durum(&dk[DK_KALKAN], g_kalkan ? 3 : 0, g_kalkan ? "Kamera kilitli" : "Kapalı");
	kutucuk_durum(&dk[DK_DND], dnd, dnd ? "Açık" : "Kapalı");
	kutucuk_durum(&dk[DK_GECE], br_auto, br_auto ? "Oto parlaklık" : "Kapalı");
	kutucuk_durum(&dk[DK_TEMA], 0, TH.ad);
	kutucuk_durum(&dk[DK_KILIT], 0, "Dokun: kilitle");
	kutucuk_durum(&dk[DK_KORUYUCU], 0, ss_tur_ad[ss_tur >= 0 && ss_tur < SS_TUR_N ? ss_tur : 1]);	/* 0.24: secili tur */
	snprintf(b, sizeof b, "%d dk sonra", ss_dk);
	kutucuk_durum(&dk[DK_OTOKOR], ss_on, ss_on ? b : "Kapalı");
	if (dm_sl_br && !lv_slider_is_dragged(dm_sl_br)) {
		int pct = br_auto ? raw_to_pct(br_cur > 0 ? br_cur : pct_to_raw(br_manual)) : br_manual;
		if (pct < 5)
			pct = 5;
		if (lv_slider_get_value(dm_sl_br) != pct)
			lv_slider_set_value(dm_sl_br, pct, LV_ANIM_OFF);
		snprintf(b, sizeof b, "%%%d", pct);
		gz_metin(dm_l_br, b);
		gz_metin(dm_l_br2, br_auto ? "Otomatik · sürüklersen manuele geçer" : "Manuel");
	}
	if (dm_sl_ses && !lv_slider_is_dragged(dm_sl_ses)) {
		if (lv_slider_get_value(dm_sl_ses) != ses_duzey)
			lv_slider_set_value(dm_sl_ses, ses_duzey, LV_ANIM_OFF);
		snprintf(b, sizeof b, ses_sessiz ? "Sessiz" : "%%%d", ses_duzey);
		gz_metin(dm_l_ses, b);
		if (dm_ses_ik)
			gz_metin(lv_obj_get_child(dm_ses_ik, 0), ses_sessiz ? LV_SYMBOL_MUTE : LV_SYMBOL_VOLUME_MAX);
		char s2[48];	/* 0.48: 0.39'dan beri codec'e uygulaniyor; A3=1 ile hoparlor calisiyor */
		if (ses_sessiz)
			snprintf(s2, sizeof s2, "Hoparlör sessiz");
		else
			snprintf(s2, sizeof s2, "Hoparlör · codec %d/7", sk_yuzde_seviye(ses_duzey));
		gz_metin(dm_l_ses2, s2);
	}
}

static void dnd_ayarla(int on)
{
	on = !!on;
	if (on == dnd)
		return;
	dnd = on;
	br_save();
	show_toast(dnd ? LV_SYMBOL_MUTE "  Rahatsız etme açık: PC bildirimleri gizlenir"
		       : LV_SYMBOL_BELL "  Rahatsız etme kapalı");
	ayar_esitle();
}

/* ------------------------------------------------------------ panel kilidi */
/* Yanlis dokunmaya karsi (guvenlik degil): tum ekrani ortur, kaydirarak acilir. Kamera/mikrofon hapi gorunur. */
static lv_obj_t *kl_obj, *kl_saat, *kl_tarih, *kl_sl;
static hap_t kl_priv;

static void kilit_ac(void)
{
	cope_at(kl_obj);
	kl_obj = kl_saat = kl_tarih = kl_sl = NULL;
	kl_priv.kap = NULL;
	if (kilitli)
		printf("kilit: kapali\n");
	kilitli = 0;
}

static int kl_bas_sol;	/* kaydirma soldaki topuzdan basladi mi (sagda tek dokunus kilidi acmasin) */

static void kl_sl_cb(lv_event_t *e)
{
	if (!kl_sl)
		return;
	if (lv_event_get_code(e) == LV_EVENT_PRESSED) {
		lv_point_t p;
		lv_area_t a;
		lv_indev_t *in = lv_indev_active();
		kl_bas_sol = 0;
		if (in) {
			lv_indev_get_point(in, &p);
			lv_obj_get_coords(kl_sl, &a);
			kl_bas_sol = p.x < a.x1 + 170;
		}
		return;
	}
	if (kl_bas_sol && lv_slider_get_value(kl_sl) >= 90) {
		kilit_ac();
		show_toast(LV_SYMBOL_OK "  Kilit açıldı");
	} else
		lv_slider_set_value(kl_sl, 0, LV_ANIM_ON);
}

static void kilit_guncelle(void)
{
	char b[96];
	if (!kl_obj)
		return;
	time_t now = time(NULL);
	struct tm tm;
	localtime_r(&now, &tm);
	if (tm.tm_year > 120) {
		snprintf(b, sizeof b, "%02d:%02d", tm.tm_hour, tm.tm_min);
		gz_metin(kl_saat, b);
		snprintf(b, sizeof b, "%d %s %s", tm.tm_mday, aylar[tm.tm_mon], gunler[tm.tm_wday]);
		gz_metin(kl_tarih, b);
	}
	if (g_kam || g_mik) {
		snprintf(b, sizeof b, "%s%s%s", g_kam ? "KAMERA AÇIK" : "", g_kam && g_mik ? " · " : "", g_mik ? "MİKROFON AÇIK" : "");
		hap_ayarla(&kl_priv, lv_color_hex(g_kam ? 0xDC2626 : 0xEA580C), 1, b);
		gizle(kl_priv.kap, 0);
	} else
		gizle(kl_priv.kap, 1);
}

static void not_kapat(void);

static void kilit_goster(void)
{
	if (kl_obj)
		return;
	dm_git(0, 0);
	not_kapat();
	if (wf_win) {
		lv_obj_delete(wf_win);	/* tick/uzaktan komut baglaminda (Wi-Fi penceresinin kendi olayindan degil) */
		wf_win = NULL;
	}
	kilitli = 1;
	printf("kilit: acik\n");
	kl_obj = lv_obj_create(lv_layer_top());
	lv_obj_remove_style_all(kl_obj);
	lv_obj_set_size(kl_obj, W, H);
	lv_obj_set_style_bg_opa(kl_obj, LV_OPA_COVER, 0);
	lv_obj_set_style_bg_color(kl_obj, C_BG, 0);
	lv_obj_set_style_text_color(kl_obj, lv_color_hex(TH.text), 0);
	lv_obj_remove_flag(kl_obj, LV_OBJ_FLAG_SCROLLABLE);	/* tiklanabilir kalir: alttaki arayuze dokunus gecmez */
	lv_obj_t *kol = duz(kl_obj, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
	lv_obj_set_flex_flow(kol, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_flex_align(kol, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	lv_obj_set_style_pad_row(kol, 4, 0);
	lv_obj_align(kol, LV_ALIGN_TOP_MID, 0, 34);
	ikon_t ik = ikon_kilit(kol, 44);
	lv_obj_set_style_margin_bottom(lv_obj_get_child(kol, 0), 30, 0);
	ikon_renk(&ik, C_DIM, C_BG);
	kl_saat = label(kol, f_clock, "--:--");
	kl_tarih = label(kol, f_l, "");
	lv_obj_set_style_text_color(kl_tarih, C_DIM, 0);
	kl_priv = hap(kol);
	lv_obj_set_style_margin_top(kl_priv.kap, 10, 0);
	lv_obj_add_flag(kl_priv.kap, LV_OBJ_FLAG_HIDDEN);
	lv_obj_remove_flag(kl_priv.kap, LV_OBJ_FLAG_CLICKABLE);

	kl_sl = lv_slider_create(kl_obj);
	lv_obj_set_size(kl_sl, 640, 84);
	lv_obj_align(kl_sl, LV_ALIGN_BOTTOM_MID, 0, -40);
	lv_obj_set_ext_click_area(kl_sl, 16);
	lv_slider_set_range(kl_sl, 0, 100);
	lv_obj_set_style_bg_color(kl_sl, lv_color_hex(TH.btn), LV_PART_MAIN);
	lv_obj_set_style_bg_opa(kl_sl, LV_OPA_COVER, LV_PART_MAIN);
	lv_obj_set_style_border_width(kl_sl, 1, LV_PART_MAIN);
	lv_obj_set_style_border_color(kl_sl, lv_color_hex(TH.sep), LV_PART_MAIN);
	lv_obj_set_style_radius(kl_sl, 42, LV_PART_MAIN);
	lv_obj_set_style_radius(kl_sl, 42, LV_PART_INDICATOR);
	lv_obj_set_style_bg_color(kl_sl, C_ACC, LV_PART_INDICATOR);
	lv_obj_set_style_bg_opa(kl_sl, LV_OPA_40, LV_PART_INDICATOR);
	lv_obj_set_style_bg_color(kl_sl, C_ACC, LV_PART_KNOB);
	lv_obj_set_style_pad_all(kl_sl, 0, LV_PART_KNOB);
	lv_obj_set_style_shadow_width(kl_sl, 0, LV_PART_KNOB);
	lv_obj_set_style_anim_duration(kl_sl, 200, 0);
	lv_obj_t *ip = label(kl_sl, f_m, "Kilidi açmak için sağa kaydır  »");
	lv_obj_set_style_text_color(ip, C_DIM, 0);
	lv_obj_center(ip);
	lv_obj_add_event_cb(kl_sl, kl_sl_cb, LV_EVENT_PRESSED, NULL);
	lv_obj_add_event_cb(kl_sl, kl_sl_cb, LV_EVENT_RELEASED, NULL);
	lv_obj_add_event_cb(kl_sl, kl_sl_cb, LV_EVENT_PRESS_LOST, NULL);
	kilit_guncelle();
}

/* ------------------------------------------------------------------ Medya */
static lv_obj_t *md_pc_durum, *md_bt_bag, *md_bt_muzik, *md_bt_kod, *md_bt_ad;
/* 0.66: PC'de calan medya (pc_medya_v1 -> PC ajani -> {"t":"ui","pc_medya":{...}}) */
static lv_obj_t *md_pc_baslik, *md_pc_sanatci;
static char pcm_baslik[160], pcm_sanatci[112], pcm_uyg[28], pcm_durum[16];
static uint32_t pcm_t;

static void md_makro_cb(lv_event_t *e)
{
	makro_gonder((const char *)lv_event_get_user_data(e));
}

static lv_obj_t *yuvarlak(lv_obj_t *par, int32_t cap, const char *sembol, int vurgu, const char *makro)
{
	lv_obj_t *b = dugme(par, sembol, cap, cap, makro ? md_makro_cb : NULL, (void *)makro, vurgu);
	lv_obj_set_style_radius(b, LV_RADIUS_CIRCLE, 0);
	return b;
}

static void medya_build(lv_obj_t *t)
{
	lv_obj_set_flex_flow(t, LV_FLEX_FLOW_ROW);
	lv_obj_set_style_pad_column(t, 14, 0);
	lv_obj_remove_flag(t, LV_OBJ_FLAG_SCROLLABLE);

	lv_obj_t *c1 = card_i(t, LV_SYMBOL_AUDIO, "BİLGİSAYAR MEDYASI");
	lv_obj_set_flex_grow(c1, 6);
	lv_obj_t *r = kutu(c1, LV_FLEX_FLOW_ROW, 40);
	lv_obj_set_flex_grow(r, 1);
	lv_obj_set_flex_align(r, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	lv_obj_set_style_pad_left(r, 12, 0);
	/* plak (birkac daire; resim/yeni varlik yok) */
	lv_obj_t *plak = daire(r, 236, lv_color_hex(0x0F172A));
	lv_obj_set_style_border_width(plak, 2, 0);
	lv_obj_set_style_border_color(plak, lv_color_hex(TH.sep), 0);
	for (int i = 0; i < 3; i++) {
		lv_obj_t *o = duz(plak, 196 - i * 36, 196 - i * 36);
		lv_obj_set_style_radius(o, LV_RADIUS_CIRCLE, 0);
		lv_obj_set_style_border_width(o, 1, 0);
		lv_obj_set_style_border_color(o, lv_color_hex(0x334155), 0);
		lv_obj_center(o);
	}
	lv_obj_t *etk = daire(plak, 84, C_ACC);
	lv_obj_set_style_bg_grad_color(etk, lv_color_hex(TH.acc2), 0);
	lv_obj_set_style_bg_grad_dir(etk, LV_GRAD_DIR_VER, 0);
	lv_obj_center(etk);
	lv_obj_center(daire(plak, 14, lv_color_hex(0x0F172A)));
	lv_obj_t *sag = kutu(r, LV_FLEX_FLOW_COLUMN, 10);	/* 0.68: 14 -> 10 */
	lv_obj_set_width(sag, 0);
	lv_obj_set_flex_grow(sag, 1);
	md_pc_baslik = label(sag, f_l, "Bilgisayardaki medya");	/* 0.66: calan baslik */
	lv_label_set_long_mode(md_pc_baslik, LV_LABEL_LONG_DOT);
	lv_obj_set_width(md_pc_baslik, LV_PCT(100));
	md_pc_sanatci = label(sag, f_m, "");
	lv_label_set_long_mode(md_pc_sanatci, LV_LABEL_LONG_DOT);
	lv_obj_set_width(md_pc_sanatci, LV_PCT(100));
	lv_obj_set_style_text_color(md_pc_sanatci, C_DIM, 0);
	lv_obj_add_flag(md_pc_sanatci, LV_OBJ_FLAG_HIDDEN);
	md_pc_durum = label(sag, f_s, "");
	lv_obj_set_style_text_color(md_pc_durum, C_DIM, 0);
	lv_obj_t *kr = kutu(sag, LV_FLEX_FLOW_ROW, 22);
	lv_obj_set_flex_align(kr, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	yuvarlak(kr, 76, LV_SYMBOL_PREV, 0, "prev");	/* 0.68: 88/112 -> 76/96 */
	yuvarlak(kr, 96, LV_SYMBOL_PLAY " " LV_SYMBOL_PAUSE, 1, "playpause");
	yuvarlak(kr, 76, LV_SYMBOL_NEXT, 0, "next");
	lv_obj_t *sr = kutu(sag, LV_FLEX_FLOW_ROW, 12);
	dugme(sr, LV_SYMBOL_VOLUME_MID "  Ses -", 160, 52, md_makro_cb, (void *)"voldown", 0);
	dugme(sr, LV_SYMBOL_MUTE "  Sessiz", 160, 52, md_makro_cb, (void *)"mute", 0);
	dugme(sr, LV_SYMBOL_VOLUME_MAX "  Ses +", 160, 52, md_makro_cb, (void *)"volup", 0);
	soluk(c1, f_s, "Tuşlar PC ajanına gider (Kısayollar ile aynı); cihazın kendi sesini değiştirmez.");

	lv_obj_t *c2 = card_i(t, LV_SYMBOL_BLUETOOTH, "TELEFON (BLUETOOTH)");
	lv_obj_set_flex_grow(c2, 4);
	md_bt_bag = label(c2, f_l, "-");
	md_bt_muzik = label(c2, f_m, "");
	md_bt_kod = label(c2, f_s, "");
	lv_obj_set_style_text_color(md_bt_kod, C_DIM, 0);
	md_bt_ad = label(c2, f_s, "");
	lv_obj_set_style_text_color(md_bt_ad, C_DIM, 0);
	lv_obj_t *br = kutu(c2, LV_FLEX_FLOW_ROW, 16);
	lv_obj_set_style_pad_top(br, 6, 0);
	lv_obj_set_flex_align(br, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	const char *sm[3] = {LV_SYMBOL_PREV, LV_SYMBOL_PLAY, LV_SYMBOL_NEXT};
	for (int i = 0; i < 3; i++)
		lv_obj_add_state(yuvarlak(br, 72, sm[i], 0, NULL), LV_STATE_DISABLED);
	lv_obj_t *uy = soluk(c2, f_s, "Telefon kontrolü yakında: MCU müzik komutları henüz doğrulanmadı, panel MCU'ya hiçbir şey göndermez.");
	lv_obj_set_style_text_color(uy, C_WARN, 0);
}

/* 0.68: baslik tek satira sigarsa kalin 40 px; sigmazsa kalin 28 px en cok IKI satir (LONG_DOT "..."). Yalniz baslik ya da
 * genislik degisince olculur (her tick'te stil yazilmasin). */
static void md_baslik_yaz(const char *t)
{
	static char son[160];
	static int32_t son_w;
	int32_t w = lv_obj_get_content_width(md_pc_baslik);
	if (w <= 0) {
		gz_metin(md_pc_baslik, t);
		return;
	}
	if (w == son_w && !strcmp(son, t))
		return;
	snprintf(son, sizeof son, "%s", t);
	son_w = w;
	const lv_font_t *f = f_l;
	lv_point_t p;
	lv_text_get_size(&p, t, f, 0, 0, w, LV_TEXT_FLAG_NONE);
	if (p.y > lv_font_get_line_height(f)) {
		f = f_mb;
		lv_text_get_size(&p, t, f, 0, 0, w, LV_TEXT_FLAG_NONE);
	}
	int32_t lh = lv_font_get_line_height(f);
	lv_obj_set_style_text_font(md_pc_baslik, f, 0);
	lv_obj_set_height(md_pc_baslik, p.y > lh ? 2 * lh : lh);
	lv_label_set_text(md_pc_baslik, t);
}

static void medya_tick(void)
{
	char b[160], ad[64] = "", yol[160];
	if (!md_bt_bag)
		return;
	int pcm_var = g_pc_ok && pcm_t && (uint32_t)(tick_ms() - pcm_t) < 90000 && pcm_durum[0] && strcmp(pcm_durum, "yok") &&
		      pcm_baslik[0];	/* 0.66 */
	if (pcm_var) {
		char s[160];
		snprintf(s, sizeof s, "%s%s%s", pcm_sanatci, pcm_sanatci[0] && pcm_uyg[0] ? " · " : "", pcm_uyg);
		md_baslik_yaz(pcm_baslik);
		gz_metin(md_pc_sanatci, s);
		lv_obj_remove_flag(md_pc_sanatci, LV_OBJ_FLAG_HIDDEN);
		int oy = !strcmp(pcm_durum, "oynuyor");
		gz_metin(md_pc_durum, oy ? LV_SYMBOL_PLAY "  Oynatılıyor" : !strcmp(pcm_durum, "duraklatildi") ? LV_SYMBOL_PAUSE "  Duraklatıldı"
			 : LV_SYMBOL_STOP "  Durdu");
		gz_renk(md_pc_durum, oy ? C_OK : C_DIM);
	} else {
		md_baslik_yaz("Bilgisayardaki medya");
		lv_obj_add_flag(md_pc_sanatci, LV_OBJ_FLAG_HIDDEN);
		gz_metin(md_pc_durum, g_pc_ok ? "PC bağlı · oynat, duraklat, şarkı geç, ses" : "PC bağlı değil · tuşlar kuyrukta bekler");
		gz_renk(md_pc_durum, g_pc_ok ? C_OK : C_DIM);
	}
	struct bt_medya m = bt_medya_oku();
	gz_metin(md_bt_bag, !m.taze ? LV_SYMBOL_BLUETOOTH "  Bilgi yok" : m.baglanti == 1 ? LV_SYMBOL_BLUETOOTH "  Telefon bağlı"
								      : LV_SYMBOL_BLUETOOTH "  Bağlı değil");
	gz_renk(md_bt_bag, m.taze && m.baglanti == 1 ? C_OK : C_DIM);
	snprintf(b, sizeof b, "Müzik: %s", bt_medya_durum_metni(&m));
	gz_metin(md_bt_muzik, b);
	if (m.muzik_olayi)
		snprintf(b, sizeof b, "Son müzik olayı %s · %.0f sn önce", m.kod, m.yas);
	else
		snprintf(b, sizeof b, "%s", m.taze ? "Müzik olayı henüz gelmedi" : "mcu-servis durum dosyası yok ya da eski");
	gz_metin(md_bt_kod, b);
	veri_yol(yol, sizeof yol, "bt-ad");
	if (dosya_oku(yol, ad, sizeof ad) > 0)
		ad[strcspn(ad, "\r\n")] = 0;
	snprintf(b, sizeof b, "Kayıtlı ad: %s", ad[0] ? ad : "VeLinux KÖLE (varsayılan)");
	gz_metin(md_bt_ad, b);
}

/* ------------------------------------------------------------------ Notlar */
static struct not_listesi notlar;
static lv_obj_t *nt_liste, *nt_ozet, *nt_bar, *nt_ta, *nt_yeni;

static void notlar_yukle(void)
{
	char yol[160];
	static char buf[NOT_MAX * (NOT_UZUN + 4)];
	veri_yol(yol, sizeof yol, "notlar.txt");
	notlar.n = 0;
	if (dosya_oku(yol, buf, sizeof buf) > 0)
		notlar_coz(&notlar, buf);
}

static void notlar_kaydet(void)
{
	char yol[160];
	static char buf[NOT_MAX * (NOT_UZUN + 4)];
	int n = notlar_yaz(&notlar, buf, sizeof buf);
	veri_yol(yol, sizeof yol, "notlar.txt");
	if (n >= 0 && !dosya_yaz_atomik(yol, buf, (size_t)n))
		sync();
	else
		show_toast(LV_SYMBOL_WARNING "  Not kaydedilemedi (/data yazılamıyor)");
}

static void notlar_ciz(void);
static void nt_ciz_async(void *u)
{
	(void)u;
	notlar_ciz();
}

static void nt_isaret_cb(lv_event_t *e)
{
	int i = (int)(uintptr_t)lv_event_get_user_data(e);
	if (i < 0 || i >= notlar.n)
		return;
	notlar.o[i].bitti = !notlar.o[i].bitti;
	notlar_kaydet();
	lv_async_call(nt_ciz_async, NULL);	/* satir kendi olayinin icinden silinmesin */
}

static void nt_sil_cb(lv_event_t *e)
{
	int i = (int)(uintptr_t)lv_event_get_user_data(e);
	not_sil(&notlar, i);
	notlar_kaydet();
	lv_async_call(nt_ciz_async, NULL);
}

static void nt_temizle_cb(lv_event_t *e)
{
	(void)e;
	int s = notlar_biteni_temizle(&notlar);
	if (s) {
		notlar_kaydet();
		lv_async_call(nt_ciz_async, NULL);
	}
	show_toast(s ? LV_SYMBOL_TRASH "  Tamamlananlar silindi" : "Tamamlanan not yok");
}

static void notlar_ciz(void)
{
	char b[64];
	if (!nt_liste)
		return;
	lv_obj_clean(nt_liste);
	for (int i = 0; i < notlar.n; i++) {
		lv_obj_t *r = lv_obj_create(nt_liste);
		lv_obj_remove_style_all(r);
		lv_obj_set_size(r, LV_PCT(100), 64);
		lv_obj_set_style_bg_opa(r, LV_OPA_COVER, 0);
		lv_obj_set_style_bg_color(r, lv_color_hex(TH.btn), 0);
		lv_obj_set_style_radius(r, 14, 0);
		lv_obj_set_style_pad_hor(r, 14, 0);
		lv_obj_set_flex_flow(r, LV_FLEX_FLOW_ROW);
		lv_obj_set_flex_align(r, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
		lv_obj_set_style_pad_column(r, 14, 0);
		lv_obj_remove_flag(r, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
		lv_obj_add_flag(r, LV_OBJ_FLAG_EVENT_BUBBLE);
		int bitti = notlar.o[i].bitti;
		lv_obj_t *k = lv_obj_create(r);	/* yuvarlak isaret kutusu */
		lv_obj_remove_style_all(k);
		lv_obj_set_size(k, 40, 40);
		lv_obj_set_style_radius(k, LV_RADIUS_CIRCLE, 0);
		lv_obj_set_style_border_width(k, 3, 0);
		lv_obj_set_style_border_color(k, C_ACC, 0);
		lv_obj_set_style_bg_opa(k, bitti ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
		lv_obj_set_style_bg_color(k, C_ACC, 0);
		lv_obj_set_ext_click_area(k, 12);
		lv_obj_remove_flag(k, LV_OBJ_FLAG_SCROLLABLE);
		lv_obj_add_event_cb(k, nt_isaret_cb, LV_EVENT_CLICKED, (void *)(uintptr_t)i);
		if (bitti) {
			lv_obj_t *ok = label(k, f_s, LV_SYMBOL_OK);
			lv_obj_set_style_text_color(ok, lv_color_white(), 0);
			lv_obj_center(ok);
		}
		lv_obj_t *m = label(r, f_m, notlar.o[i].metin);
		lv_obj_set_width(m, 0);
		lv_obj_set_flex_grow(m, 1);
		lv_label_set_long_mode(m, LV_LABEL_LONG_DOT);
		lv_obj_set_height(m, lv_font_get_line_height(f_m));
		if (bitti) {
			lv_obj_set_style_text_color(m, C_DIM, 0);
			lv_obj_set_style_text_decor(m, LV_TEXT_DECOR_STRIKETHROUGH, 0);
		}
		lv_obj_t *d = lv_button_create(r);
		lv_obj_remove_style_all(d);
		lv_obj_set_size(d, 52, 48);
		lv_obj_set_style_radius(d, 12, 0);
		lv_obj_set_style_bg_color(d, lv_color_hex(0xDC2626), LV_STATE_PRESSED);
		lv_obj_set_style_bg_opa(d, LV_OPA_COVER, LV_STATE_PRESSED);
		lv_obj_add_event_cb(d, nt_sil_cb, LV_EVENT_CLICKED, (void *)(uintptr_t)i);
		lv_obj_t *dl = label(d, f_mb, LV_SYMBOL_TRASH);
		lv_obj_set_style_text_color(dl, C_DIM, 0);
		lv_obj_center(dl);
	}
	if (!notlar.n) {
		lv_obj_t *l = soluk(nt_liste, f_m, "Henüz not yok.\nSağdaki “Yeni not” düğmesiyle ekle; bitenleri yuvarlağa dokunarak işaretle.");
		lv_obj_set_style_text_line_space(l, 8, 0);
		lv_obj_set_style_pad_top(l, 20, 0);
	}
	int bit = notlar_biten(&notlar);
	snprintf(b, sizeof b, "%d / %d", bit, notlar.n);
	gz_metin(nt_ozet, b);
	lv_bar_set_value(nt_bar, notlar.n ? bit * 100 / notlar.n : 0, LV_ANIM_OFF);
	if (nt_yeni) {
		if (notlar.n >= NOT_MAX)
			lv_obj_add_state(nt_yeni, LV_STATE_DISABLED);
		else
			lv_obj_remove_state(nt_yeni, LV_STATE_DISABLED);
	}
}

static void not_kapat(void)
{
	cope_at(not_win);
	not_win = nt_ta = NULL;
}

static void not_kaydet_kapat(void)
{
	if (!nt_ta)
		return;
	int r = not_ekle(&notlar, lv_textarea_get_text(nt_ta));
	not_kapat();
	if (r == 0) {
		notlar_kaydet();
		notlar_ciz();
		show_toast(LV_SYMBOL_OK "  Not eklendi");
	} else if (notlar.n >= NOT_MAX)
		show_toast(LV_SYMBOL_WARNING "  Liste dolu (en fazla 16 not)");
}

static void nt_kb_cb(lv_event_t *e)
{
	lv_event_code_t c = lv_event_get_code(e);
	if (c == LV_EVENT_READY)
		not_kaydet_kapat();
	else if (c == LV_EVENT_CANCEL)
		not_kapat();
}

static void nt_btn_cb(lv_event_t *e)
{
	if (lv_event_get_user_data(e))
		not_kaydet_kapat();
	else
		not_kapat();
}

/* Turkce Q klavye (lv_keyboard kucuk/buyuk harf haritalari; "1#" ozel karakterler LVGL varsayilani) */
#define KB_TUS(w) (w)
#define KB_OZEL(w) (LV_KEYBOARD_CTRL_BUTTON_FLAGS | (w))
#define KB_KOYU(w) (LV_BUTTONMATRIX_CTRL_CHECKED | (w))
static const char *const kb_tr_kucuk[] = {
	"1#", "q", "w", "e", "r", "t", "y", "u", "ı", "o", "p", "ğ", "ü", LV_SYMBOL_BACKSPACE, "\n",
	"ABC", "a", "s", "d", "f", "g", "h", "j", "k", "l", "ş", "i", ",", "\n",
	"-", "z", "x", "c", "v", "b", "n", "m", "ö", "ç", ".", "?", "\n",
	LV_SYMBOL_CLOSE, LV_SYMBOL_LEFT, " ", LV_SYMBOL_RIGHT, LV_SYMBOL_OK, ""};
static const char *const kb_tr_buyuk[] = {
	"1#", "Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "Ğ", "Ü", LV_SYMBOL_BACKSPACE, "\n",
	"abc", "A", "S", "D", "F", "G", "H", "J", "K", "L", "Ş", "İ", ",", "\n",
	"-", "Z", "X", "C", "V", "B", "N", "M", "Ö", "Ç", ".", "?", "\n",
	LV_SYMBOL_CLOSE, LV_SYMBOL_LEFT, " ", LV_SYMBOL_RIGHT, LV_SYMBOL_OK, ""};
static const lv_buttonmatrix_ctrl_t kb_tr_ctrl[] = {
	KB_OZEL(5), KB_TUS(4), KB_TUS(4), KB_TUS(4), KB_TUS(4), KB_TUS(4), KB_TUS(4), KB_TUS(4), KB_TUS(4), KB_TUS(4),
	KB_TUS(4), KB_TUS(4), KB_TUS(4), KB_KOYU(6),
	KB_OZEL(5), KB_TUS(4), KB_TUS(4), KB_TUS(4), KB_TUS(4), KB_TUS(4), KB_TUS(4), KB_TUS(4), KB_TUS(4), KB_TUS(4),
	KB_TUS(4), KB_TUS(4), KB_KOYU(4),
	KB_KOYU(4), KB_TUS(4), KB_TUS(4), KB_TUS(4), KB_TUS(4), KB_TUS(4), KB_TUS(4), KB_TUS(4), KB_TUS(4), KB_TUS(4),
	KB_KOYU(4), KB_KOYU(4),
	KB_OZEL(2), KB_KOYU(2), KB_TUS(6), KB_KOYU(2), KB_OZEL(2)};

static void not_ac(void)
{
	if (not_win || kilitli)
		return;
	dm_git(0, 0);
	not_win = lv_obj_create(lv_layer_top());
	lv_obj_remove_style_all(not_win);
	lv_obj_set_size(not_win, W, H);
	lv_obj_set_style_bg_opa(not_win, LV_OPA_COVER, 0);
	lv_obj_set_style_bg_color(not_win, C_BG, 0);
	lv_obj_set_style_text_color(not_win, lv_color_hex(TH.text), 0);
	lv_obj_set_style_pad_all(not_win, 16, 0);
	lv_obj_remove_flag(not_win, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_t *ust = kutu(not_win, LV_FLEX_FLOW_ROW, 12);
	lv_obj_set_height(ust, 60);
	lv_obj_set_flex_align(ust, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	lv_obj_t *bs = label(ust, f_l, LV_SYMBOL_EDIT "  Yeni not");
	lv_obj_set_flex_grow(bs, 1);
	dugme(ust, LV_SYMBOL_CLOSE "  İptal", 180, 56, nt_btn_cb, NULL, 0);
	dugme(ust, LV_SYMBOL_OK "  Kaydet", 200, 56, nt_btn_cb, (void *)1, 1);
	nt_ta = lv_textarea_create(not_win);
	lv_obj_set_size(nt_ta, LV_PCT(100), LV_SIZE_CONTENT);
	lv_obj_set_y(nt_ta, 72);
	lv_textarea_set_one_line(nt_ta, true);
	lv_textarea_set_max_length(nt_ta, NOT_KARAKTER);
	lv_textarea_set_placeholder_text(nt_ta, "Örn. Süt al, faturayı öde, Ayşe'yi ara");
	lv_obj_set_style_text_font(nt_ta, f_l, 0);
	lv_obj_set_style_bg_color(nt_ta, C_CARD, 0);
	lv_obj_set_style_border_color(nt_ta, C_ACC, LV_STATE_FOCUSED);
	lv_obj_add_state(nt_ta, LV_STATE_FOCUSED);	/* imlec gorunsun */
	lv_obj_t *kb = lv_keyboard_create(not_win);
	lv_obj_set_size(kb, LV_PCT(100), 330);
	lv_obj_align(kb, LV_ALIGN_BOTTOM_MID, 0, 0);
	lv_obj_set_style_text_font(kb, f_mb, 0);
	lv_keyboard_set_map(kb, LV_KEYBOARD_MODE_TEXT_LOWER, kb_tr_kucuk, kb_tr_ctrl);
	lv_keyboard_set_map(kb, LV_KEYBOARD_MODE_TEXT_UPPER, kb_tr_buyuk, kb_tr_ctrl);
	lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_TEXT_LOWER);
	lv_keyboard_set_textarea(kb, nt_ta);
	lv_obj_add_event_cb(kb, nt_kb_cb, LV_EVENT_READY, NULL);
	lv_obj_add_event_cb(kb, nt_kb_cb, LV_EVENT_CANCEL, NULL);
}

static void nt_yeni_cb(lv_event_t *e)
{
	(void)e;
	not_ac();
}

/* 0.69: Asistan > "Soru yaz": serbest metin soru (Turkce klavye) -> cihazdaki Gemini (as_cihazda_sor) */
static lv_obj_t *as_win, *as_ta;

static void as_soru_kapat(void)
{
	cope_at(as_win);
	as_win = as_ta = NULL;
}

static void as_soru_gonder(void)
{
	if (!as_ta)
		return;
	char s[AS_SORU_MAX + 1];
	snprintf(s, sizeof s, "%s", lv_textarea_get_text(as_ta));
	as_soru_kapat();
	const char *b = s;
	while (*b == ' ')
		b++;
	if (*b)
		as_cihazda_sor(b);
}

static void as_kb_cb(lv_event_t *e)
{
	lv_event_code_t c = lv_event_get_code(e);
	if (c == LV_EVENT_READY)
		as_soru_gonder();
	else if (c == LV_EVENT_CANCEL)
		as_soru_kapat();
}

static void as_win_btn_cb(lv_event_t *e)
{
	if (lv_event_get_user_data(e))
		as_soru_gonder();
	else
		as_soru_kapat();
}

static void as_soru_cb(lv_event_t *e)
{
	(void)e;
	if (as_win || kilitli)
		return;
	dm_git(0, 0);
	as_win = lv_obj_create(lv_layer_top());
	lv_obj_remove_style_all(as_win);
	lv_obj_set_size(as_win, W, H);
	lv_obj_set_style_bg_opa(as_win, LV_OPA_COVER, 0);
	lv_obj_set_style_bg_color(as_win, C_BG, 0);
	lv_obj_set_style_text_color(as_win, lv_color_hex(TH.text), 0);
	lv_obj_set_style_pad_all(as_win, 16, 0);
	lv_obj_remove_flag(as_win, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_t *ust = kutu(as_win, LV_FLEX_FLOW_ROW, 12);
	lv_obj_set_height(ust, 60);
	lv_obj_set_flex_align(ust, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	lv_obj_t *bs = label(ust, f_l, LV_SYMBOL_EDIT "  Asistana sor");
	lv_obj_set_flex_grow(bs, 1);
	dugme(ust, LV_SYMBOL_CLOSE "  İptal", 180, 56, as_win_btn_cb, NULL, 0);
	dugme(ust, LV_SYMBOL_OK "  Sor", 200, 56, as_win_btn_cb, (void *)1, 1);
	as_ta = lv_textarea_create(as_win);
	lv_obj_set_size(as_ta, LV_PCT(100), LV_SIZE_CONTENT);
	lv_obj_set_y(as_ta, 72);
	lv_textarea_set_one_line(as_ta, true);
	lv_textarea_set_max_length(as_ta, AS_SORU_MAX / 2);	/* UTF-8 Turkce harf 2 bayt: AS_SORU_MAX'a sigar */
	lv_textarea_set_placeholder_text(as_ta, "Örn. Yarın Ankara'da yağmur var mı?");
	lv_obj_set_style_text_font(as_ta, f_l, 0);
	lv_obj_set_style_bg_color(as_ta, C_CARD, 0);
	lv_obj_set_style_border_color(as_ta, C_ACC, LV_STATE_FOCUSED);
	lv_obj_add_state(as_ta, LV_STATE_FOCUSED);	/* imlec gorunsun */
	lv_obj_t *kb = lv_keyboard_create(as_win);
	lv_obj_set_size(kb, LV_PCT(100), 330);
	lv_obj_align(kb, LV_ALIGN_BOTTOM_MID, 0, 0);
	lv_obj_set_style_text_font(kb, f_mb, 0);
	lv_keyboard_set_map(kb, LV_KEYBOARD_MODE_TEXT_LOWER, kb_tr_kucuk, kb_tr_ctrl);
	lv_keyboard_set_map(kb, LV_KEYBOARD_MODE_TEXT_UPPER, kb_tr_buyuk, kb_tr_ctrl);
	lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_TEXT_LOWER);
	lv_keyboard_set_textarea(kb, as_ta);
	lv_obj_add_event_cb(kb, as_kb_cb, LV_EVENT_READY, NULL);
	lv_obj_add_event_cb(kb, as_kb_cb, LV_EVENT_CANCEL, NULL);
}

static void notlar_build(lv_obj_t *t)
{
	lv_obj_set_flex_flow(t, LV_FLEX_FLOW_ROW);
	lv_obj_set_style_pad_column(t, 14, 0);
	lv_obj_remove_flag(t, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_t *c1 = card_i(t, LV_SYMBOL_LIST, "YAPILACAKLAR");
	lv_obj_set_flex_grow(c1, 7);
	nt_liste = lv_obj_create(c1);
	lv_obj_remove_style_all(nt_liste);
	lv_obj_set_width(nt_liste, LV_PCT(100));
	lv_obj_set_flex_grow(nt_liste, 1);
	lv_obj_set_flex_flow(nt_liste, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_style_pad_row(nt_liste, 10, 0);
	lv_obj_set_scroll_dir(nt_liste, LV_DIR_VER);
	lv_obj_set_scrollbar_mode(nt_liste, LV_SCROLLBAR_MODE_ACTIVE);

	lv_obj_t *c2 = card_i(t, LV_SYMBOL_OK, "ÖZET");
	lv_obj_set_flex_grow(c2, 3);
	nt_ozet = label(c2, f_xl, "0 / 0");
	soluk(c2, f_s, "tamamlandı");
	nt_bar = lv_bar_create(c2);
	lv_obj_set_size(nt_bar, LV_PCT(100), 14);
	lv_bar_set_range(nt_bar, 0, 100);
	lv_obj_set_style_bg_color(nt_bar, lv_color_hex(TH.sep), LV_PART_MAIN);
	lv_obj_set_style_bg_opa(nt_bar, LV_OPA_COVER, LV_PART_MAIN);
	lv_obj_set_style_bg_color(nt_bar, C_OK, LV_PART_INDICATOR);
	lv_obj_t *bos = duz(c2, 1, 1);
	lv_obj_set_flex_grow(bos, 1);
	nt_yeni = dugme(c2, LV_SYMBOL_PLUS "  Yeni not", LV_PCT(100), 72, nt_yeni_cb, NULL, 1);
	dugme(c2, LV_SYMBOL_TRASH "  Bitenleri temizle", LV_PCT(100), 60, nt_temizle_cb, NULL, 0);
	soluk(c2, f_s, "En fazla 16 not · /data/notlar.txt");
	notlar_ciz();
}

/* ---------------------------------------------------------------- Monitor */
#define MON_N 60
static int8_t mon_cpu_g[MON_N], mon_ram_g[MON_N];
static int mon_bas, mon_say, mon_cpu = -1, mon_ram = -1, mon_onceki_ok, mon_net_ok;
static double mon_sic = -1000, mon_rx_hiz, mon_tx_hiz;
static long mon_rss = -1, mon_top_kb, mon_uyg_kb;
static unsigned long long mon_rx, mon_tx;
static uint32_t mon_net_ms;
static struct cpu_ornek mon_onceki;
static lv_obj_t *mn_chart, *mn_cpu, *mn_ram, *mn_sic, *mn_v[9];
static lv_chart_series_t *mn_s_cpu, *mn_s_ram;

static void mon_ornekle(void)
{
	static char buf[4096];
	char yol[200];
	struct cpu_ornek o;

	proc_yol(yol, sizeof yol, "/proc/stat");
	if (dosya_oku(yol, buf, sizeof buf) > 0 && !cpu_ornek_coz(buf, &o)) {
		mon_cpu = mon_onceki_ok ? cpu_yuzde(&mon_onceki, &o) : -1;
		mon_onceki = o;
		mon_onceki_ok = 1;
	} else
		mon_cpu = -1;
	struct bellek_durum b;
	proc_yol(yol, sizeof yol, "/proc/meminfo");
	if (dosya_oku(yol, buf, sizeof buf) > 0 && !bellek_coz(buf, &b)) {
		mon_top_kb = b.toplam_kb;
		mon_uyg_kb = b.uygun_kb;
		mon_ram = (int)((b.toplam_kb - b.uygun_kb) * 100 / b.toplam_kb);
	} else
		mon_ram = -1;
	proc_yol(yol, sizeof yol, "/sys/class/thermal/thermal_zone0/temp");
	if (dosya_oku(yol, buf, sizeof buf) <= 0 || sicaklik_coz(buf, &mon_sic))
		mon_sic = -1000;
	proc_yol(yol, sizeof yol, "/proc/self/status");
	mon_rss = dosya_oku(yol, buf, sizeof buf) > 0 ? vmrss_kb(buf) : -1;
	unsigned long long rx, tx;
	proc_yol(yol, sizeof yol, "/proc/net/dev");
	uint32_t ms = tick_ms();
	if (dosya_oku(yol, buf, sizeof buf) > 0 && !netdev_coz(buf, "wlan0", &rx, &tx)) {
		if (mon_net_ok && rx >= mon_rx && tx >= mon_tx && ms - mon_net_ms > 0) {
			mon_rx_hiz = (double)(rx - mon_rx) * 1000.0 / (ms - mon_net_ms);
			mon_tx_hiz = (double)(tx - mon_tx) * 1000.0 / (ms - mon_net_ms);
		}
		mon_rx = rx;
		mon_tx = tx;
		mon_net_ms = ms;
		mon_net_ok = 1;
	} else
		mon_net_ok = 0;
	if (mon_cpu < 0 && !mon_say)
		return;	/* ilk ornekte islemci yuzdesi yok (fark gerekir): gecmise yazma */
	int i = (mon_bas + mon_say) % MON_N;
	if (mon_say == MON_N)
		mon_bas = (mon_bas + 1) % MON_N;
	else
		mon_say++;
	/* gecmiste yalniz gecerli degerler (LVGL 9.2 cizgi grafigi LV_CHART_POINT_NONE ile isaretli tasma yapiyor) */
	int on = (i + MON_N - 1) % MON_N, ilk = mon_say == 1;
	mon_cpu_g[i] = (int8_t)(mon_cpu >= 0 ? mon_cpu : ilk ? 0 : mon_cpu_g[on]);
	mon_ram_g[i] = (int8_t)(mon_ram >= 0 ? mon_ram : ilk ? 0 : mon_ram_g[on]);
}

/* grafigi halka tampondan yeniden doldur (sekmeye girerken; gizliyken grafik guncellenmez) */
static void mon_chart_doldur(void)
{
	if (!mn_chart)
		return;
	/* ilk dakikada nokta sayisi = ornek sayisi (bos nokta yok); sonra sabit 60, kaydirmali */
	uint32_t n = mon_say < 2 ? 2 : (uint32_t)mon_say;
	if (lv_chart_get_point_count(mn_chart) != n)
		lv_chart_set_point_count(mn_chart, n);
	int32_t *yc = lv_chart_get_y_array(mn_chart, mn_s_cpu), *yr = lv_chart_get_y_array(mn_chart, mn_s_ram);
	for (uint32_t k = 0; k < n; k++) {
		int j = (mon_bas + (int)(k < (uint32_t)mon_say ? k : 0)) % MON_N;
		yc[k] = mon_say ? mon_cpu_g[j] : 0;
		yr[k] = mon_say ? mon_ram_g[j] : 0;
	}
	lv_chart_set_x_start_point(mn_chart, mn_s_cpu, 0);
	lv_chart_set_x_start_point(mn_chart, mn_s_ram, 0);
	lv_chart_refresh(mn_chart);
}

static void depolama(char *o, size_t n, const char *yol, int bagli_mi)
{
	char p[200];
	struct statvfs v;
	struct stat a, b;
	proc_yol(p, sizeof p, yol);
	if (proc_kok[0]) {	/* yalniz test kancasi: <kok><yol>/.ornek-boyut "kullanilan_mb toplam_mb" */
		char y2[230], buf[64];
		double k1, k2;
		snprintf(y2, sizeof y2, "%.200s/.ornek-boyut", p);
		if (dosya_oku(y2, buf, sizeof buf) > 0 && sscanf(buf, "%lf %lf", &k1, &k2) == 2) {
			char t1[24], t2[24];
			kd_boyut(t1, sizeof t1, k1);
			kd_boyut(t2, sizeof t2, k2);
			snprintf(o, n, "%s / %s", t1, t2);
			return;
		}
	}
	if (bagli_mi) {	/* bagli degilse ust dizinle ayni aygit */
		char ust[208];
		snprintf(ust, sizeof ust, "%.200s/..", p);
		if (stat(p, &a) || stat(ust, &b) || a.st_dev == b.st_dev) {
			snprintf(o, n, "takılı değil");
			return;
		}
	}
	if (statvfs(p, &v) || !v.f_blocks) {
		snprintf(o, n, "-");
		return;
	}
	double top = (double)v.f_blocks * v.f_frsize / 1048576.0, bos = (double)v.f_bavail * v.f_frsize / 1048576.0;
	char t1[24], t2[24];
	kd_boyut(t1, sizeof t1, top - bos);
	kd_boyut(t2, sizeof t2, top);
	snprintf(o, n, "%s / %s", t1, t2);
}

static void mon_ciz(void)
{
	char b[96], h1[24], h2[24], yol[200], buf[128];
	if (!mn_chart)
		return;
	if (mon_cpu >= 0)
		snprintf(b, sizeof b, "%%%d", mon_cpu);
	else
		snprintf(b, sizeof b, "-");
	gz_metin(mn_cpu, b);
	if (mon_ram >= 0)
		snprintf(b, sizeof b, "%%%d", mon_ram);
	else
		snprintf(b, sizeof b, "-");
	gz_metin(mn_ram, b);
	if (mon_sic > -999)
		snprintf(b, sizeof b, "%.0f°C", mon_sic);
	else
		snprintf(b, sizeof b, "-");
	gz_metin(mn_sic, b);
	gz_renk(mn_sic, mon_sic > 75 ? C_BAD : mon_sic > 60 ? C_WARN : lv_color_hex(TH.text));
	if (mon_say < MON_N || lv_chart_get_point_count(mn_chart) != MON_N)
		mon_chart_doldur();	/* ilk dakika: nokta sayisi buyuyor */
	else {
		int son = (mon_bas + mon_say - 1) % MON_N;
		lv_chart_set_next_value(mn_chart, mn_s_cpu, mon_cpu_g[son]);
		lv_chart_set_next_value(mn_chart, mn_s_ram, mon_ram_g[son]);
	}

	double up = -1;
	proc_yol(yol, sizeof yol, "/proc/uptime");
	if (dosya_oku(yol, buf, sizeof buf) > 0)
		up = strtod(buf, NULL);
	sure_bicim(b, sizeof b, (long)up);
	gz_metin(mn_v[0], up >= 0 ? b : "-");
	proc_yol(yol, sizeof yol, "/proc/loadavg");
	if (dosya_oku(yol, buf, sizeof buf) > 0) {
		double l1, l5, l15;
		if (sscanf(buf, "%lf %lf %lf", &l1, &l5, &l15) == 3)
			snprintf(b, sizeof b, "%.2f · %.2f · %.2f", l1, l5, l15);
		else
			snprintf(b, sizeof b, "-");
	} else
		snprintf(b, sizeof b, "-");
	gz_metin(mn_v[1], b);
	if (mon_top_kb > 0)
		snprintf(b, sizeof b, "%ld / %ld MB kullanımda", (mon_top_kb - mon_uyg_kb) / 1024, mon_top_kb / 1024);
	else
		snprintf(b, sizeof b, "-");
	gz_metin(mn_v[2], b);
	if (mon_rss > 0)
		snprintf(b, sizeof b, "%.1f MB", mon_rss / 1024.0);
	else
		snprintf(b, sizeof b, "-");
	gz_metin(mn_v[3], b);
	if (strcmp(g_ip, "-") && wifi_duzey(g_dbm))
		snprintf(b, sizeof b, "%s · %d dBm", wl_ssid, g_dbm);
	else
		snprintf(b, sizeof b, "%s", strcmp(g_ip, "-") ? wl_ssid : "bağlı değil");
	gz_metin(mn_v[4], b);
	if (mon_net_ok) {
		hiz_bicim(h1, sizeof h1, mon_rx_hiz);
		hiz_bicim(h2, sizeof h2, mon_tx_hiz);
		snprintf(b, sizeof b, LV_SYMBOL_DOWNLOAD " %s   " LV_SYMBOL_UPLOAD " %s", h1, h2);
	} else
		snprintf(b, sizeof b, "-");
	gz_metin(mn_v[5], b);
	depolama(b, sizeof b, veri_dizin[0] == '/' && !proc_kok[0] ? veri_dizin : "/data", 0);
	gz_metin(mn_v[6], b);
	depolama(b, sizeof b, "/mnt/sd", 1);
	gz_metin(mn_v[7], b);
	depolama(b, sizeof b, "/tmp", 0);
	gz_metin(mn_v[8], b);
}

static lv_obj_t *mn_kutu(lv_obj_t *par, const char *ad, lv_obj_t **deger, lv_color_t nokta)
{
	lv_obj_t *k = lv_obj_create(par);
	lv_obj_remove_style_all(k);
	lv_obj_set_height(k, LV_SIZE_CONTENT);
	lv_obj_set_width(k, 0);
	lv_obj_set_flex_grow(k, 1);
	lv_obj_set_style_bg_opa(k, LV_OPA_COVER, 0);
	lv_obj_set_style_bg_color(k, lv_color_hex(TH.btn), 0);
	lv_obj_set_style_radius(k, 16, 0);
	lv_obj_set_style_pad_all(k, 14, 0);
	lv_obj_set_flex_flow(k, LV_FLEX_FLOW_COLUMN);
	lv_obj_remove_flag(k, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
	lv_obj_t *r = kutu(k, LV_FLEX_FLOW_ROW, 8);
	lv_obj_set_flex_align(r, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	daire(r, 12, nokta);
	lv_obj_t *a = label(r, f_s, ad);
	lv_obj_set_style_text_color(a, C_DIM, 0);
	*deger = label(k, f_l, "-");
	return k;
}

static void monitor_build(lv_obj_t *t)
{
	lv_obj_set_flex_flow(t, LV_FLEX_FLOW_ROW);
	lv_obj_set_style_pad_column(t, 14, 0);
	lv_obj_remove_flag(t, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_t *c1 = card_i(t, LV_SYMBOL_CHARGE, "İŞLEMCİ & BELLEK · SON 60 SN");
	lv_obj_set_flex_grow(c1, 6);
	lv_obj_t *r = kutu(c1, LV_FLEX_FLOW_ROW, 12);
	mn_kutu(r, "İşlemci", &mn_cpu, C_ACC);
	mn_kutu(r, "Bellek", &mn_ram, C_OK);
	mn_kutu(r, "Sıcaklık", &mn_sic, C_WARN);
	mn_chart = lv_chart_create(c1);
	lv_obj_set_width(mn_chart, LV_PCT(100));
	lv_obj_set_flex_grow(mn_chart, 1);
	lv_chart_set_type(mn_chart, LV_CHART_TYPE_LINE);
	lv_chart_set_point_count(mn_chart, 2);	/* mon_chart_doldur ornek sayisina gore buyutur */
	lv_chart_set_update_mode(mn_chart, LV_CHART_UPDATE_MODE_SHIFT);
	lv_chart_set_range(mn_chart, LV_CHART_AXIS_PRIMARY_Y, 0, 100);
	lv_chart_set_div_line_count(mn_chart, 5, 0);
	lv_obj_set_style_bg_opa(mn_chart, LV_OPA_TRANSP, 0);
	lv_obj_set_style_border_width(mn_chart, 0, 0);
	lv_obj_set_style_pad_all(mn_chart, 4, 0);
	lv_obj_set_style_line_color(mn_chart, lv_color_hex(TH.sep), LV_PART_MAIN);
	lv_obj_set_style_line_width(mn_chart, 3, LV_PART_ITEMS);
	lv_obj_set_style_size(mn_chart, 0, 0, LV_PART_INDICATOR);	/* nokta cizme: yalniz cizgi */
	lv_obj_remove_flag(mn_chart, LV_OBJ_FLAG_CLICKABLE);
	mn_s_cpu = lv_chart_add_series(mn_chart, C_ACC, LV_CHART_AXIS_PRIMARY_Y);
	mn_s_ram = lv_chart_add_series(mn_chart, C_OK, LV_CHART_AXIS_PRIMARY_Y);

	lv_obj_t *c2 = card_i(t, LV_SYMBOL_SETTINGS, "SİSTEM");
	lv_obj_set_flex_grow(c2, 4);
	lv_obj_set_style_pad_row(c2, 6, 0);
	static const char *const adlar[9] = {"Çalışma süresi", "Yük (1/5/15 dk)", "Bellek", "Panel belleği", "Wi-Fi",
					     "Ağ hızı", "/data (ayarlar)", "SD kart", "/tmp (RAM)"};
	for (int i = 0; i < 9; i++)
		mn_v[i] = gz_satir(c2, adlar[i]);
	mon_chart_doldur();
}

/* ---------------------------------------------------------------- 0.67 PC Sistem (pc_sistem_v1.h, PC: pc_sistem_v1.py) */
static lv_obj_t *pcs_grafik(lv_obj_t *par, lv_chart_type_t tur, int32_t ust)
{
	lv_obj_t *c = lv_chart_create(par);
	lv_chart_set_type(c, tur);
	lv_chart_set_point_count(c, 2);
	lv_chart_set_update_mode(c, LV_CHART_UPDATE_MODE_SHIFT);
	lv_chart_set_range(c, LV_CHART_AXIS_PRIMARY_Y, 0, ust);
	lv_chart_set_div_line_count(c, 3, 0);
	lv_obj_set_style_bg_opa(c, LV_OPA_TRANSP, 0);
	lv_obj_set_style_border_width(c, 0, 0);
	lv_obj_set_style_pad_all(c, 2, 0);
	lv_obj_set_style_line_color(c, lv_color_hex(TH.sep), LV_PART_MAIN);
	lv_obj_set_style_line_width(c, 2, LV_PART_ITEMS);
	lv_obj_set_style_size(c, 0, 0, LV_PART_INDICATOR);
	lv_obj_remove_flag(c, LV_OBJ_FLAG_CLICKABLE);
	return c;
}

/* kart: ust satir buyuk deger + ayrinti, altta grafik alani (dondurulur) */
static lv_obj_t *pcs_kart(lv_obj_t *par, const char *ikon, const char *baslik, int buyume, lv_obj_t **deger, lv_obj_t **ayrinti)
{
	lv_obj_t *c = card_i(par, ikon, baslik);
	lv_obj_set_flex_grow(c, buyume);
	lv_obj_set_width(c, 0);
	lv_obj_set_style_pad_all(c, 14, 0);
	lv_obj_set_style_pad_row(c, 6, 0);
	lv_obj_t *r = kutu(c, LV_FLEX_FLOW_ROW, 12);
	lv_obj_set_flex_align(r, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	*deger = label(r, f_l, "-");
	*ayrinti = label(r, f_s, "");
	lv_obj_set_style_text_color(*ayrinti, C_DIM, 0);
	lv_obj_set_flex_grow(*ayrinti, 1);
	lv_label_set_long_mode(*ayrinti, LV_LABEL_LONG_WRAP);
	return c;
}

static lv_obj_t *pcs_alan(lv_obj_t *kart)	/* grafiklerin satiri: kalan yukseklik */
{
	lv_obj_t *a = kutu(kart, LV_FLEX_FLOW_ROW, 10);
	lv_obj_set_flex_grow(a, 1);
	return a;
}

static lv_obj_t *pcs_grafik_ekle(lv_obj_t *alan, lv_chart_type_t tur, int32_t ust, int buyume)
{
	lv_obj_t *c = pcs_grafik(alan, tur, ust);
	lv_obj_set_height(c, LV_PCT(100));
	lv_obj_set_width(c, 0);
	lv_obj_set_flex_grow(c, buyume);
	return c;
}

static void pcs_build(lv_obj_t *t)
{
	lv_obj_set_flex_flow(t, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_style_pad_row(t, 8, 0);
	lv_obj_remove_flag(t, LV_OBJ_FLAG_SCROLLABLE);
	pcs_l_durum = soluk(t, f_s, "PC ölçümleri bekleniyor");
	lv_obj_t *r1 = kutu(t, LV_FLEX_FLOW_ROW, 12), *r2 = kutu(t, LV_FLEX_FLOW_ROW, 12);
	lv_obj_set_flex_grow(r1, 1);
	lv_obj_set_flex_grow(r2, 1);
	lv_obj_t *a;
	/* ust sira */
	a = pcs_alan(pcs_kart(r1, LV_SYMBOL_CHARGE, "İŞLEMCİ", 5, &pcs_l_cpu, &pcs_l_cpu_d));
	pcs_ch_cpu = pcs_grafik_ekle(a, LV_CHART_TYPE_LINE, 100, 3);
	pcs_s_cpu = lv_chart_add_series(pcs_ch_cpu, C_ACC, LV_CHART_AXIS_PRIMARY_Y);
	pcs_ch_cek = pcs_grafik_ekle(a, LV_CHART_TYPE_BAR, 100, 2);
	lv_obj_set_style_pad_column(pcs_ch_cek, 2, 0);
	pcs_s_cek = lv_chart_add_series(pcs_ch_cek, C_ACC, LV_CHART_AXIS_PRIMARY_Y);
	a = pcs_alan(pcs_kart(r1, LV_SYMBOL_IMAGE, "EKRAN KARTI", 4, &pcs_l_gpu, &pcs_l_gpu_d));
	pcs_ch_gpu = pcs_grafik_ekle(a, LV_CHART_TYPE_LINE, 100, 1);
	pcs_s_gpu = lv_chart_add_series(pcs_ch_gpu, C_WARN, LV_CHART_AXIS_PRIMARY_Y);
	a = pcs_alan(pcs_kart(r1, LV_SYMBOL_LIST, "BELLEK", 3, &pcs_l_ram, &pcs_l_ram_d));
	pcs_ch_ram = pcs_grafik_ekle(a, LV_CHART_TYPE_LINE, 100, 1);
	pcs_s_ram = lv_chart_add_series(pcs_ch_ram, C_OK, LV_CHART_AXIS_PRIMARY_Y);
	/* alt sira */
	lv_obj_t *kd = pcs_kart(r2, LV_SYMBOL_DRIVE, "DİSK", 4, &pcs_l_disk, &pcs_l_disk_d);
	for (int i = 0; i < PCS_SUR; i++) {
		lv_obj_t *sr = kutu(kd, LV_FLEX_FLOW_ROW, 8);
		lv_obj_set_flex_align(sr, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
		pcs_sur_l[i] = label(sr, f_s, "");
		lv_obj_set_width(pcs_sur_l[i], 170);
		pcs_sur_bar[i] = lv_bar_create(sr);
		lv_obj_set_height(pcs_sur_bar[i], 10);
		lv_obj_set_width(pcs_sur_bar[i], 0);
		lv_obj_set_flex_grow(pcs_sur_bar[i], 1);
		lv_bar_set_range(pcs_sur_bar[i], 0, 100);
		lv_obj_set_style_bg_color(pcs_sur_bar[i], lv_color_hex(TH.btn), LV_PART_MAIN);
		lv_obj_set_style_bg_color(pcs_sur_bar[i], C_ACC, LV_PART_INDICATOR);
		lv_obj_add_flag(sr, LV_OBJ_FLAG_HIDDEN);
	}
	a = pcs_alan(kd);
	pcs_ch_disk = pcs_grafik_ekle(a, LV_CHART_TYPE_LINE, 100, 1);
	pcs_s_disk = lv_chart_add_series(pcs_ch_disk, C_ACC, LV_CHART_AXIS_PRIMARY_Y);
	a = pcs_alan(pcs_kart(r2, LV_SYMBOL_WIFI, "AĞ", 3, &pcs_l_ag, &pcs_l_ag_d));
	pcs_ch_ag = pcs_grafik_ekle(a, LV_CHART_TYPE_LINE, 64, 1);
	pcs_s_in = lv_chart_add_series(pcs_ch_ag, C_OK, LV_CHART_AXIS_PRIMARY_Y);
	pcs_s_out = lv_chart_add_series(pcs_ch_ag, C_ACC, LV_CHART_AXIS_PRIMARY_Y);
	lv_obj_t *ks = card_i(r2, LV_SYMBOL_SETTINGS, "SÜREÇLER · EN ÇOK İŞLEMCİ  |  EN ÇOK BELLEK");	/* 0.73 */
	lv_obj_set_flex_grow(ks, 5);
	lv_obj_set_width(ks, 0);
	lv_obj_set_style_pad_all(ks, 14, 0);
	lv_obj_set_style_pad_row(ks, 4, 0);
	for (int i = 0; i < PCS_ILK; i++) {
		lv_obj_t *sr = kutu(ks, LV_FLEX_FLOW_ROW, 8);
		lv_obj_set_flex_align(sr, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
		for (int k = 0; k < 4; k += 2) {	/* 0.73: [islemci ad | %]  [bellek ad | MB] */
			pcs_ilk_l[i][k] = label(sr, f_s, "");
			lv_obj_set_width(pcs_ilk_l[i][k], 0);
			lv_obj_set_flex_grow(pcs_ilk_l[i][k], 1);
			lv_label_set_long_mode(pcs_ilk_l[i][k], LV_LABEL_LONG_DOT);
			pcs_ilk_l[i][k + 1] = label(sr, f_s, "");
			lv_obj_set_width(pcs_ilk_l[i][k + 1], k ? 100 : 72);
			lv_obj_set_style_text_align(pcs_ilk_l[i][k + 1], LV_TEXT_ALIGN_RIGHT, 0);
			lv_obj_set_style_text_color(pcs_ilk_l[i][k + 1], C_DIM, 0);
			if (!k)
				lv_obj_set_style_pad_right(pcs_ilk_l[i][k + 1], 14, 0);
		}
	}
}

/* gecmis -> seri (sec: 0 cpu, 1 ram, 2 gpu, 3 disk, 4 indirme, 5 yukleme) */
static void pcs_seri(lv_obj_t *ch, lv_chart_series_t *s, int sec)
{
	uint32_t n = pcs.say < 2 ? 2 : (uint32_t)pcs.say;
	if (lv_chart_get_point_count(ch) != n)
		lv_chart_set_point_count(ch, n);
	int32_t *y = lv_chart_get_y_array(ch, s);
	for (uint32_t k = 0; k < n; k++) {
		int j = pcs_gecmis_i(&pcs, k < (uint32_t)pcs.say ? (int)k : 0);
		y[k] = !pcs.say ? 0 : sec == 0 ? pcs.g_cpu[j] : sec == 1 ? pcs.g_ram[j] : sec == 2 ? pcs.g_gpu[j]
			: sec == 3 ? pcs.g_disk[j] : sec == 4 ? pcs.g_in[j] : pcs.g_out[j];
	}
	lv_chart_set_x_start_point(ch, s, 0);
}

static void pcs_ciz(void)
{
	char b[200], h1[24], h2[24];
	if (!pcs_l_cpu)
		return;
	if (!pcs.n) {
		gz_metin(pcs_l_durum, g_pc_ok ? "PC ölçümleri bekleniyor · PC'de pc_sistem_v1 çalışmalı" : "PC bağlı değil");
		return;
	}
	int taze = pcs_t && (uint32_t)(tick_ms() - pcs_t) < 10000;
	snprintf(b, sizeof b, "%s · %s · %d süreç · %d iş parçacığı · açık %d sa %d dk", pcs.ad, taze ? "canlı, 2 sn" : "VERİ ESKİ",
		 pcs.surec, pcs.ip, pcs.calisma / 3600, pcs.calisma / 60 % 60);
	gz_metin(pcs_l_durum, b);
	gz_renk(pcs_l_durum, taze ? C_DIM : C_WARN);
	/* islemci */
	int tepe = 0;
	for (int i = 0; i < pcs.cek_n; i++)
		if (pcs.cek[i] > tepe)
			tepe = pcs.cek[i];
	snprintf(b, sizeof b, "%%%d", pcs.cpu);
	gz_metin(pcs_l_cpu, b);
	snprintf(b, sizeof b, "%d çekirdek · en yüksek %%%d\nson 3 dk | çekirdekler", pcs.cek_n, tepe);
	gz_metin(pcs_l_cpu_d, b);
	if (pcs.cek_n) {
		if (lv_chart_get_point_count(pcs_ch_cek) != (uint32_t)pcs.cek_n)
			lv_chart_set_point_count(pcs_ch_cek, (uint32_t)pcs.cek_n);
		int32_t *y = lv_chart_get_y_array(pcs_ch_cek, pcs_s_cek);
		for (int i = 0; i < pcs.cek_n; i++)
			y[i] = pcs.cek[i];
		lv_chart_refresh(pcs_ch_cek);
	}
	pcs_seri(pcs_ch_cpu, pcs_s_cpu, 0);
	lv_chart_refresh(pcs_ch_cpu);
	/* ekran karti */
	if (pcs.gpu_var && pcs.gpu >= 0) {
		snprintf(b, sizeof b, "%%%d", pcs.gpu);
		gz_metin(pcs_l_gpu, b);
		snprintf(b, sizeof b, "%s · VRAM %d.%d / %d GB\n%d °C · %d.%d W · %d MHz · fan %%%d", pcs.gpu_ad, pcs.vk / 1024,
			 (pcs.vk % 1024) * 10 / 1024, pcs.vt / 1024, pcs.gsic, pcs.gguc10 / 10, pcs.gguc10 % 10, pcs.gsaat, pcs.gfan);
	} else {
		gz_metin(pcs_l_gpu, "-");
		snprintf(b, sizeof b, "Ekran kartı bilgisi yok (nvidia-smi)");
	}
	gz_metin(pcs_l_gpu_d, b);
	pcs_seri(pcs_ch_gpu, pcs_s_gpu, 2);
	lv_chart_refresh(pcs_ch_gpu);
	/* bellek */
	snprintf(b, sizeof b, "%%%d", pcs.ram_y);
	gz_metin(pcs_l_ram, b);
	snprintf(b, sizeof b, "%d.%d / %d GB kullanımda\nayrılan %d.%d / %d GB", pcs.ram_kul / 1024, (pcs.ram_kul % 1024) * 10 / 1024,
		 (pcs.ram_top + 512) / 1024, pcs.cm / 1024, (pcs.cm % 1024) * 10 / 1024, (pcs.cml + 512) / 1024);
	gz_metin(pcs_l_ram_d, b);
	pcs_seri(pcs_ch_ram, pcs_s_ram, 1);
	lv_chart_refresh(pcs_ch_ram);
	/* disk */
	snprintf(b, sizeof b, "%%%d", pcs.etkin);
	gz_metin(pcs_l_disk, b);
	pcs_hiz(h1, sizeof h1, pcs.oku);
	pcs_hiz(h2, sizeof h2, pcs.yaz);
	snprintf(b, sizeof b, "etkin · okuma %s · yazma %s", h1, h2);
	gz_metin(pcs_l_disk_d, b);
	for (int i = 0; i < PCS_SUR; i++) {
		lv_obj_t *sr = lv_obj_get_parent(pcs_sur_l[i]);
		if (i >= pcs.sur_n) {
			lv_obj_add_flag(sr, LV_OBJ_FLAG_HIDDEN);
			continue;
		}
		lv_obj_remove_flag(sr, LV_OBJ_FLAG_HIDDEN);
		snprintf(b, sizeof b, "%s  %d / %d GB", pcs.sur_ad[i], pcs.sur_kul[i], pcs.sur_top[i]);
		gz_metin(pcs_sur_l[i], b);
		int y = pcs.sur_top[i] > 0 ? pcs.sur_kul[i] * 100 / pcs.sur_top[i] : 0;
		lv_bar_set_value(pcs_sur_bar[i], y, LV_ANIM_OFF);
		lv_obj_set_style_bg_color(pcs_sur_bar[i], y >= 90 ? C_WARN : C_ACC, LV_PART_INDICATOR);
	}
	pcs_seri(pcs_ch_disk, pcs_s_disk, 3);
	lv_chart_refresh(pcs_ch_disk);
	/* ag */
	pcs_hiz(h1, sizeof h1, pcs.ag_in);
	pcs_hiz(h2, sizeof h2, pcs.ag_out);
	snprintf(b, sizeof b, LV_SYMBOL_DOWN " %s", h1);
	gz_metin(pcs_l_ag, b);
	snprintf(b, sizeof b, LV_SYMBOL_UP " %s yükleme\nyeşil indirme, mavi yükleme", h2);
	gz_metin(pcs_l_ag_d, b);
	int32_t ust = pcs_ag_tepe(&pcs);
	lv_chart_set_range(pcs_ch_ag, LV_CHART_AXIS_PRIMARY_Y, 0, ust + ust / 5);
	pcs_seri(pcs_ch_ag, pcs_s_in, 4);
	pcs_seri(pcs_ch_ag, pcs_s_out, 5);
	lv_chart_refresh(pcs_ch_ag);
	/* surecler */
	for (int i = 0; i < PCS_ILK; i++) {	/* 0.73: sol en cok islemci, sag en cok bellek */
		if (i < pcs.ilk_n) {
			gz_metin(pcs_ilk_l[i][0], pcs.ilk_ad[i]);
			snprintf(b, sizeof b, "%%%d.%d", pcs.ilk_cpu10[i] / 10, pcs.ilk_cpu10[i] % 10);
			gz_metin(pcs_ilk_l[i][1], b);
		} else {
			gz_metin(pcs_ilk_l[i][0], "");
			gz_metin(pcs_ilk_l[i][1], "");
		}
		if (i < pcs.ram_n) {
			gz_metin(pcs_ilk_l[i][2], pcs.ram_ad[i]);
			snprintf(b, sizeof b, "%d MB", pcs.ram_mb[i]);
			gz_metin(pcs_ilk_l[i][3], b);
		} else {
			gz_metin(pcs_ilk_l[i][2], !i && pcs.ilk_n && !pcs.ram_n ? "—" : "");	/* eski PC hizmeti: ilk_ram yok */
			gz_metin(pcs_ilk_l[i][3], "");
		}
	}
}

/* ---------------------------------------------------------------- Ayarlar */
enum { AY_EKRAN, AY_KORUYUCU, AY_SES, AY_KOLE, AY_BAGLANTI, AY_GIZLILIK, AY_SISTEM, AY_N };	/* 0.30: KOLE; 0.54: KORUYUCU */
static const char *const ay_adi[AY_N] = {"ekran", "koruyucu", "ses", "kole", "baglanti", "gizlilik", "sistem"};
static lv_obj_t *ay_tur[SS_TUR_N];	/* 0.24, 0.40: 7 tur */
static lv_obj_t *ay_nav[AY_N], *ay_bol[AY_N], *ay_dk[4], *ay_sl_ses, *ay_l_ses, *ay_sw_sessiz, *ay_sw_kalkan, *ay_sw_dnd,
	*ay_l_kam, *ay_l_wifi, *ay_l_pc, *ay_l_btad, *ay_l_bld, *ay_l_codec, *ay_l_sktest;	/* 0.39 */
static int ay_secili;
static const int ay_dk_deger[4] = {1, 3, 5, 10};

static void ayar_bolum_sec(int i)
{
	if (i < 0 || i >= AY_N)
		return;
	ay_secili = i;
	for (int k = 0; k < AY_N; k++) {
		if (!ay_nav[k])
			continue;
		gizle(ay_bol[k], k != i);
		bg_renk(ay_nav[k], k == i ? C_ACC : lv_color_hex(TH.btn));
		lv_obj_set_style_bg_opa(ay_nav[k], k == i ? LV_OPA_COVER : LV_OPA_TRANSP, 0);
		for (int c = 0; c < 2; c++) {	/* 0.53: simge + yazi */
			lv_obj_t *ch = lv_obj_get_child(ay_nav[k], c);
			if (ch)
				gz_renk(ch, k == i ? lv_color_white() : lv_color_hex(TH.text));
		}
	}
}

static void ay_nav_cb(lv_event_t *e)
{
	ayar_bolum_sec((int)(uintptr_t)lv_event_get_user_data(e));
}

static void ay_dk_cb(lv_event_t *e)
{
	ss_dk = ay_dk_deger[(int)(uintptr_t)lv_event_get_user_data(e) & 3];
	br_save();
	ayar_esitle();
}

/* 0.24: koruyucu turu (Saat / Robot goz / Karisik) */
static void ay_tur_cb(lv_event_t *e)
{
	int t = (int)(uintptr_t)lv_event_get_user_data(e);
	if (t < 0 || t >= SS_TUR_N || t == ss_tur)
		return;
	ss_tur = t;
	br_save();
	ayar_esitle();
	printf("koruyucu turu: %s\n", ss_tur_kod[ss_tur]);
}

/* 0.40: secili koruyucuyu hemen goster (cerceve acik olsa da secili tur) */
static void ay_ss_onizle_cb(lv_event_t *e)
{
	(void)e;
	if (!ss_obj && !pc_full)
		ss_show_tur(ss_tur);
}

static void ay_ses_cb(lv_event_t *e)
{
	if (!ay_sl_ses)
		return;
	ses_duzey = lv_slider_get_value(ay_sl_ses);
	ses_sessiz = 0;
	lv_label_set_text_fmt(ay_l_ses, "%%%d", ses_duzey);
	if (lv_event_get_code(e) == LV_EVENT_RELEASED) {
		br_save();
		ayar_esitle();
	}
}

static void ay_sessiz_cb(lv_event_t *e)
{
	ses_sessiz = lv_obj_has_state(lv_event_get_target(e), LV_STATE_CHECKED);
	br_save();
	ayar_esitle();
}


/* ------------------------------------------------------------ 0.43 acilis animasyonu */
/* VeLinux (isletim sistemi) -> KOLE (urun). Siyah ortu lv_layer_top'ta; V isareti iki lv_line (yuvarlak uc), nokta bir
 * daire, "Ve"/"Linux" A8 maskeler (boyanir), KOLE logo_tam. Yalniz yaprak nesnelerin opa/konum/olcek degerleri degisir
 * (katman yok). Zaman cizelgesi ms: 0-380 sol kol, 300-680 sag kol, 600-900 nokta, 750-1150 yazi, 1750-2050 VeLinux
 * soner, 1950-2350 KOLE belirir, 2750-3050 ortu soner, 3050 bitti. Dokunma hemen bitirir. */
static lv_obj_t *aa_obj, *aa_sol, *aa_sag, *aa_nokta, *aa_ve, *aa_li, *aa_kole;
static lv_point_precise_t aa_p1[2], aa_p2[2];
static uint32_t aa_t0;
static lv_timer_t *aa_tmr;
#define AA_X 460	/* V kutusu (220x220) sol ust */
#define AA_Y 190

static void aa_bitir(void)
{
	if (aa_tmr)
		lv_timer_delete(aa_tmr);
	aa_tmr = NULL;
	if (aa_obj) {
		lv_obj_delete(aa_obj);
		aa_obj = NULL;
		printf("acilis animasyonu: bitti (%u ms)\n", (unsigned)(tick_ms() - aa_t0));
	}
}

/* yeniden kurulum (tema): ortu lv_layer_top ile silindi; yalniz zamanlayici durur, isaretciler sifirlanir */
static void aa_temizle(void)
{
	if (aa_tmr)
		lv_timer_delete(aa_tmr);
	aa_tmr = NULL;
	aa_obj = NULL;
}

static void aa_dokun_cb(lv_event_t *e)
{
	(void)e;
	aa_bitir();
}

/* 0..1024 yavaslayan gecis (ease-out kubik) */
static int32_t aa_k(int32_t t, int32_t sure)
{
	if (t <= 0)
		return 0;
	if (t >= sure)
		return 1024;
	float x = (float)t / (float)sure, y = 1.0f - (1.0f - x) * (1.0f - x) * (1.0f - x);
	return (int32_t)(y * 1024.0f);
}

static lv_opa_t aa_opa(int32_t k)
{
	return (lv_opa_t)(k * 255 / 1024);
}

static void aa_tmr_cb(lv_timer_t *t)
{
	(void)t;
	if (!aa_obj)
		return;
	int32_t e = (int32_t)(tick_ms() - aa_t0) * 10 / 16;	/* 0.47: 1.6x yavas (~4.9 sn) */
	const int32_t sx = AA_X + 44, sy = AA_Y + 53, ax = AA_X + 103, ay = AA_Y + 176, gx = AA_X + 145, gy = AA_Y + 97;
	int32_t k1 = aa_k(e, 380), k2 = aa_k(e - 300, 380), k3 = aa_k(e - 600, 300), k4 = aa_k(e - 750, 400);
	int32_t sn = 1024 - aa_k(e - 1750, 300), kb = aa_k(e - 1950, 400), so = 1024 - aa_k(e - 2750, 300);
	aa_p1[1].x = sx + (ax - sx) * k1 / 1024;
	aa_p1[1].y = sy + (ay - sy) * k1 / 1024;
	aa_p2[1].x = ax + (gx - ax) * k2 / 1024;
	aa_p2[1].y = ay + (gy - ay) * k2 / 1024;
	lv_line_set_points(aa_sol, aa_p1, 2);
	lv_line_set_points(aa_sag, aa_p2, 2);
	lv_obj_set_style_line_opa(aa_sol, k1 ? aa_opa(sn) : LV_OPA_TRANSP, 0);
	lv_obj_set_style_line_opa(aa_sag, k2 ? aa_opa(sn) : LV_OPA_TRANSP, 0);
	int32_t cap = 38 * k3 / 1024 + (k3 > 0 && k3 < 1024 ? (int32_t)(10.0f * sinf(3.14159f * (float)k3 / 1024.0f)) : 0);
	lv_obj_set_size(aa_nokta, cap, cap);
	lv_obj_set_pos(aa_nokta, AA_X + 172 - cap / 2, AA_Y + 48 - cap / 2);
	lv_obj_set_style_bg_opa(aa_nokta, aa_opa(sn), 0);
	int32_t kay = 30 - 30 * k4 / 1024;
	lv_obj_set_x(aa_ve, AA_X + 260 + kay);
	lv_obj_set_x(aa_li, AA_X + 260 + kay + (int32_t)velinux_ve.header.w);
	lv_obj_set_style_image_opa(aa_ve, aa_opa(k4 * sn / 1024), 0);
	lv_obj_set_style_image_opa(aa_li, aa_opa(k4 * sn / 1024), 0);
	lv_image_set_scale(aa_kole, (uint32_t)(208 + 48 * kb / 1024));
	lv_obj_set_style_image_opa(aa_kole, aa_opa(kb * so / 1024), 0);
	lv_obj_set_style_bg_opa(aa_obj, aa_opa(so), 0);
	if (e >= 3050)
		aa_bitir();
}

static lv_obj_t *aa_cizgi(lv_point_precise_t *p, int32_t x, int32_t y)
{
	p[0].x = p[1].x = x;
	p[0].y = p[1].y = y;
	lv_obj_t *l = lv_line_create(aa_obj);
	lv_line_set_points(l, p, 2);
	lv_obj_set_pos(l, 0, 0);
	lv_obj_set_style_line_width(l, 28, 0);
	lv_obj_set_style_line_rounded(l, true, 0);
	lv_obj_set_style_line_color(l, lv_color_hex(0xE11D48), 0);
	lv_obj_set_style_line_opa(l, LV_OPA_TRANSP, 0);
	lv_obj_remove_flag(l, LV_OBJ_FLAG_CLICKABLE);
	return l;
}

static lv_obj_t *aa_resim(const lv_image_dsc_t *src, int32_t x, int32_t y, uint32_t boya)
{
	lv_obj_t *im = lv_image_create(aa_obj);
	lv_image_set_src(im, src);
	lv_obj_set_pos(im, x, y);
	if (boya)
		lv_obj_set_style_image_recolor(im, lv_color_hex(boya), 0);	/* A8 maske rengi */
	lv_obj_set_style_image_opa(im, LV_OPA_TRANSP, 0);
	lv_obj_remove_flag(im, LV_OBJ_FLAG_CLICKABLE);
	return im;
}

static void acilis_animasyonu(void)
{
	const char *z = getenv("PANEL_ACILIS");
	if (z && z[0] == '0')
		return;
	aa_obj = lv_obj_create(lv_layer_top());
	lv_obj_remove_style_all(aa_obj);
	lv_obj_set_size(aa_obj, W, H);
	lv_obj_set_style_bg_color(aa_obj, lv_color_hex(0x050505), 0);
	lv_obj_set_style_bg_opa(aa_obj, LV_OPA_COVER, 0);
	lv_obj_remove_flag(aa_obj, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_add_flag(aa_obj, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_add_event_cb(aa_obj, aa_dokun_cb, LV_EVENT_PRESSED, NULL);
	aa_sol = aa_cizgi(aa_p1, AA_X + 44, AA_Y + 53);
	aa_sag = aa_cizgi(aa_p2, AA_X + 103, AA_Y + 176);
	aa_nokta = lv_obj_create(aa_obj);
	lv_obj_remove_style_all(aa_nokta);
	lv_obj_set_style_radius(aa_nokta, LV_RADIUS_CIRCLE, 0);
	lv_obj_set_style_bg_color(aa_nokta, lv_color_hex(0xF97316), 0);
	lv_obj_set_style_bg_opa(aa_nokta, LV_OPA_COVER, 0);
	lv_obj_set_size(aa_nokta, 0, 0);
	lv_obj_remove_flag(aa_nokta, LV_OBJ_FLAG_CLICKABLE);
	int32_t ty = AA_Y + 110 - (int32_t)velinux_ve.header.h / 2;
	aa_ve = aa_resim(&velinux_ve, AA_X + 290, ty, 0xEEECE8);
	aa_li = aa_resim(&velinux_linux, AA_X + 290 + (int32_t)velinux_ve.header.w, ty, 0xE11D48);
	aa_kole = aa_resim(&logo_tam, (W - (int32_t)logo_tam.header.w) / 2, (H - (int32_t)logo_tam.header.h) / 2, 0);
	lv_image_set_pivot(aa_kole, (int32_t)logo_tam.header.w / 2, (int32_t)logo_tam.header.h / 2);
	aa_t0 = tick_ms();
	aa_tmr = lv_timer_create(aa_tmr_cb, 30, NULL);
	printf("acilis animasyonu: basladi\n");
}

/* 0.42 hizmet sagligi (Ayarlar > Sistem). comm 15 karakterde kesildigi icin /proc/<pid>/cmdline argv0 adi kullanilir. */
typedef struct {
	const char *ad, *onek;
	int istege;	/* 1: calismamasi normal (istenince acilir) */
} hz_tanim_t;
static const hz_tanim_t hz_tanim[] = {
	{"Kamera kaydı", "kamera-kayit-v", 0},	{"Kamera köprüsü", "kole-kopru-v", 0},
	{"Cihaz merkezi", "kole-cihaz-merkez-v", 0}, {"Tailscale", "tailscaled", 0},
	{"Hey Köle", "kole-asistan-v", 0},	{"MCU hizmeti", "mcu-servis-v", 0},
	{"Wi-Fi (wpa)", "wpa_supplicant", 0},	{"PC ikinci ekran", "wifi-ekran-v", 1},
};
#define HZ_N ((int)(sizeof hz_tanim / sizeof *hz_tanim))
static lv_obj_t *hz_nokta[HZ_N], *hz_l[HZ_N], *hz_l_ozet;
static uint32_t hz_son;

static void hz_tara(int pid[HZ_N], char ad[HZ_N][40])
{
	char yol[300], c[200];
	struct dirent *e;
	memset(pid, 0, sizeof(int) * HZ_N);
	snprintf(yol, sizeof yol, "%s/proc", proc_kok);
	DIR *dr = opendir(yol);
	while (dr && (e = readdir(dr))) {
		if (e->d_name[0] < '1' || e->d_name[0] > '9')
			continue;
		snprintf(yol, sizeof yol, "%s/proc/%.20s/cmdline", proc_kok, e->d_name);
		if (dosya_oku(yol, c, sizeof c) <= 0 || !c[0])
			continue;
		const char *b = strrchr(c, '/');
		b = b ? b + 1 : c;
		for (int i = 0; i < HZ_N; i++)
			if (!pid[i] && !strncmp(b, hz_tanim[i].onek, strlen(hz_tanim[i].onek))) {
				pid[i] = atoi(e->d_name);
				const char *v = strrchr(b, '-');	/* kisa surum: "kamera-kayit-v7" -> "v7" */
				if (v && v[1] == 'v' && v[2] >= '0' && v[2] <= '9')
					snprintf(ad[i], 40, "%.8s", v + 1);
				else
					snprintf(ad[i], 40, "çalışıyor");
			}
	}
	if (dr)
		closedir(dr);
}

/* SD'deki en yuksek kole-gemini-vN (Telegram botu ve Pano verisi bunu kullanir, autorun v29+) */
static int hz_gemini_surum(void)
{
	DIR *dr = opendir("/mnt/sd/velinux/asistan");
	struct dirent *e;
	int en = 0;
	while (dr && (e = readdir(dr))) {
		int v;
		char son;
		if (sscanf(e->d_name, "kole-gemini-v%d%c", &v, &son) == 1 && v > en)
			en = v;
	}
	if (dr)
		closedir(dr);
	return en;
}

static void hz_guncelle(void)
{
	int pid[HZ_N];
	char ad[HZ_N][40], b[300], t[200];
	if (!hz_l_ozet)
		return;
	hz_tara(pid, ad);
	static int son_n = -1;
	int n = 0;
	for (int i = 0; i < HZ_N; i++)
		n += pid[i] != 0;
	if (n != son_n)
		printf("hizmetler: %d/%d calisiyor\n", n, HZ_N);
	son_n = n;
	for (int i = 0; i < HZ_N; i++) {
		if (pid[i])
			snprintf(b, sizeof b, "%.40s · %.39s", hz_tanim[i].ad, ad[i]);
		else
			snprintf(b, sizeof b, "%s · %s", hz_tanim[i].ad, hz_tanim[i].istege ? "istenince" : "KAPALI");
		gz_metin(hz_l[i], b);
		lv_obj_set_style_bg_color(hz_nokta[i], pid[i] ? C_OK : hz_tanim[i].istege ? C_DIM : C_BAD, 0);
	}
	int g = hz_gemini_surum();
	char bot[64], sl[64], ar[32] = "?", kr[32] = "?";
	if (g)
		snprintf(bot, sizeof bot, "Telegram botu kole-gemini-v%d", g);
	else
		snprintf(bot, sizeof bot, "Telegram botu SD'de yok");
	struct stat st;
	if (!stat("/tmp/saglik.log", &st)) {
		long yas = (long)(time(NULL) - st.st_mtime);
		snprintf(sl, sizeof sl, "sağlık satırı %ld sn önce", yas < 0 ? 0 : yas);
	} else
		snprintf(sl, sizeof sl, "sağlık satırı yok");
	if (dosya_oku("/tmp/autorun.log", t, sizeof t) > 0) {
		const char *a = strstr(t, "autorun v");
		if (a)
			snprintf(ar, sizeof ar, "%.*s", (int)strcspn(a + 8, " \n"), a + 8);
	}
	proc_yol(b, sizeof b, "/proc/version");
	if (dosya_oku(b, t, sizeof t) > 0) {
		const char *k = strchr(t, '#');
		if (k)
			snprintf(kr, sizeof kr, "%.*s", (int)strcspn(k, " \n"), k);
	}
	snprintf(b, sizeof b, "%.63s · %.63s\nautorun %.31s · çekirdek %.31s", bot, sl, ar, kr);
	gz_metin(hz_l_ozet, b);
}

/* 0.39 ses karti: codec okumasi + test sonucu (Ayarlar gorunurken saniyede bir) */
static void ses_kart_tick(void)
{
	char b[256];
	if (!ay_l_codec)
		return;
	sk_durum_metni(b, sizeof b, proc_kok);
	gz_metin(ay_l_codec, b);
	const char *t = sk_son_test == 0 ? "Test tonu çalındı (440 Hz, 1.2 sn)." : sk_test_metni();	/* 0.48: A3 uyarisi eskidi */
	gz_metin(ay_l_sktest, t[0] ? t : "Düzey dahili codec kazancıdır (0-7); bilgisayarın ve MCU'nun sesine dokunmaz.");
}

static void ses_test_baslat(const char *kim)
{
	int r = sk_test_baslat(ses_duzey, ses_sessiz);
	printf("ses testi (%s): %s\n", kim, r == 0 ? "basladi" : r == -1 ? "zaten suruyor" : r == -2 ? "sessiz, reddedildi" :
	       "fork hatasi");
	ses_kart_tick();
}

static void ay_sktest_cb(lv_event_t *e)
{
	(void)e;
	ses_test_baslat("dokunma");
}

static void ay_dnd_cb(lv_event_t *e)
{
	dnd_ayarla(lv_obj_has_state(lv_event_get_target(e), LV_STATE_CHECKED));
}

static void ay_git_cb(lv_event_t *e)
{
	int hedef = (int)(uintptr_t)lv_event_get_user_data(e);
	if (hedef == 100)
		kilit_goster();
	else if (hedef == 101) {
		if (!ss_obj && !pc_full)
			ss_show();
	} else if (hedef == 102)
		dm_git(1, 1);
	else if (tabview)
		lv_tabview_set_active(tabview, (uint32_t)hedef, LV_ANIM_OFF);
}

static lv_obj_t *ay_bolum(lv_obj_t *par, const char *ikon, const char *t)
{
	lv_obj_t *b = kutu(par, LV_FLEX_FLOW_COLUMN, 12);
	lv_obj_set_size(b, LV_PCT(100), LV_PCT(100));
	lv_obj_add_flag(b, LV_OBJ_FLAG_SCROLLABLE);	/* 0.54: tasan satirlar ulasilamaz kalmasin (Ekran 0.44'ten beri tasiyordu) */
	lv_obj_set_scroll_dir(b, LV_DIR_VER);
	lv_obj_set_scrollbar_mode(b, LV_SCROLLBAR_MODE_ACTIVE);
	baslik_satiri(b, ikon, t);
	return b;
}

static lv_obj_t *ay_alt_kart(lv_obj_t *par, const char *ikon, const char *t)
{
	lv_obj_t *k = lv_obj_create(par);
	lv_obj_remove_style_all(k);
	lv_obj_set_height(k, LV_PCT(100));
	lv_obj_set_width(k, 0);
	lv_obj_set_flex_grow(k, 1);
	lv_obj_set_style_bg_opa(k, LV_OPA_COVER, 0);
	lv_obj_set_style_bg_color(k, C_BG, 0);	/* dugmeler (TH.btn) kart zemininden ayrilsin */
	lv_obj_set_style_border_width(k, 1, 0);
	lv_obj_set_style_border_color(k, lv_color_hex(TH.sep), 0);
	lv_obj_set_style_radius(k, 18, 0);
	lv_obj_set_style_pad_all(k, 16, 0);
	lv_obj_set_flex_flow(k, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_style_pad_row(k, 8, 0);
	lv_obj_remove_flag(k, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);
	lv_obj_t *r = kutu(k, LV_FLEX_FLOW_ROW, 10);
	lv_obj_set_flex_align(r, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	lv_obj_t *i = label(r, f_mb, ikon);
	lv_obj_set_style_text_color(i, C_ACC, 0);
	label(r, f_mb, t);
	return k;
}

/* ------------------------------------------------------- 0.30 Ayarlar > KOLE */
static lv_obj_t *kb_agiz[KY_AGIZ_N], *kb_goz[KY_GOZ_N], *kb_bakis[KY_BAKIS_N], *kb_renk[KY_RENK_N], *kb_kar[KY_KAR_N];

static void kb_temizle(void)	/* yeniden kurulum: dugmeler ekranla silindi */
{
	memset(kb_agiz, 0, sizeof kb_agiz);
	memset(kb_goz, 0, sizeof kb_goz);
	memset(kb_bakis, 0, sizeof kb_bakis);
	memset(kb_renk, 0, sizeof kb_renk);
	memset(kb_kar, 0, sizeof kb_kar);
}

static void kb_boya(lv_obj_t **b, int n, int sec)
{
	for (int i = 0; i < n; i++) {
		if (!b[i])
			continue;
		bg_renk(b[i], i == sec ? C_ACC : lv_color_hex(TH.btn));
		lv_obj_t *c = lv_obj_get_child(b[i], 0);
		if (c)
			gz_renk(c, i == sec ? lv_color_white() : lv_color_hex(TH.text));
	}
}

static void kb_esitle(void)
{
	kb_boya(kb_agiz, KY_AGIZ_N, ky_agiz_tur);
	kb_boya(kb_goz, KY_GOZ_N, ky_goz_tur);
	kb_boya(kb_bakis, KY_BAKIS_N, ky_bakis);
	kb_boya(kb_kar, KY_KAR_N, ky_karakter);
	for (int i = 0; i < KY_RENK_N; i++)
		if (kb_renk[i]) {
			lv_obj_set_style_border_width(kb_renk[i], i == ky_renk ? 5 : 2, 0);
			lv_obj_set_style_border_color(kb_renk[i], i == ky_renk ? lv_color_white() : lv_color_hex(TH.sep), 0);
		}
}

static void kb_onizle(void)
{
	ky_konus(ky_onizle_metin[ky_karakter], 0);
}

static void kb_sec_cb(lv_event_t *e)
{
	uintptr_t u = (uintptr_t)lv_event_get_user_data(e);
	int grup = (int)(u >> 8), i = (int)(u & 255);
	switch (grup) {
	case 0: ky_agiz_tur = i; break;
	case 1: ky_goz_tur = i; break;
	case 2: ky_bakis = i; break;
	case 3: ky_renk = i; break;
	case 4: ky_karakter = i; break;
	default: return;
	}
	ky_yuz_kaydet();
	kb_esitle();
	if (ky_obj)
		lv_obj_invalidate(ky_obj);
	printf("kole yuzu: agiz %d goz %d bakis %d renk %d karakter %d\n", ky_agiz_tur, ky_goz_tur, ky_bakis, ky_renk, ky_karakter);
}

static void kb_onizle_cb(lv_event_t *e)
{
	(void)e;
	kb_onizle();
}

#define KB_BASLIK_W 190	/* 0.53: secenekler her satirda ayni sutundan baslar (saga yasli satirlar kayiyordu) */
#define KB_DUGME_W 190
static lv_obj_t *kb_satir(lv_obj_t *b, const char *baslik)
{
	lv_obj_t *r = kutu(b, LV_FLEX_FLOW_ROW, 12);
	lv_obj_set_flex_align(r, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	lv_obj_t *l = label(r, f_mb, baslik);
	lv_obj_set_width(l, KB_BASLIK_W);
	lv_obj_t *k = kutu(r, LV_FLEX_FLOW_ROW, 8);
	lv_obj_set_width(k, LV_SIZE_CONTENT);
	lv_obj_set_flex_align(k, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	return k;
}

static void kb_satir_dugmeler(lv_obj_t *b, const char *baslik, const char *const *ad, int n, int grup, lv_obj_t **cikti, int32_t w)
{
	lv_obj_t *k = kb_satir(b, baslik);
	for (int i = 0; i < n; i++) {
		cikti[i] = dugme(k, ad[i], w, 50, kb_sec_cb, (void *)(uintptr_t)(grup << 8 | i), 0);
		lv_obj_set_style_pad_hor(cikti[i], 4, 0);	/* 0.68: "Gulumseme" kenara dayaniyordu */
	}
}

static void kole_bolumu(lv_obj_t *b)
{
	lv_obj_add_flag(b, LV_OBJ_FLAG_SCROLLABLE);	/* kucuk yazitipinde tasarsa kaydirilabilsin */
	lv_obj_set_style_pad_row(b, 8, 0);
	lv_obj_t *hk = kb_satir(b, "Hey Köle");	/* 0.53 */
	dugme(hk, LV_SYMBOL_AUDIO "  Sesimi öğret", 250, 50, ky_ogret_cb, NULL, 0);	/* 0.68: 230 -> 250 */
	label(hk, f_mb, "Dinleme");	/* 0.75: anahtar etiketsizdi */
	ky_sw_asistan = anahtar(hk, ky_asistan, ky_asistan_cb);
	lv_obj_t *bs = duz(hk, 24, 1);
	(void)bs;
	label(hk, f_mb, "Altyazı");
	anahtar(hk, ky_altyazi, ky_altyazi_cb);
	kb_satir_dugmeler(b, "Ağız", ky_agiz_ad, KY_AGIZ_N, 0, kb_agiz, KB_DUGME_W);
	kb_satir_dugmeler(b, "Gözler", ky_goz_ad, KY_GOZ_N, 1, kb_goz, KB_DUGME_W);
	kb_satir_dugmeler(b, "Göz hareketi", ky_bakis_ad, KY_BAKIS_N, 2, kb_bakis, KB_DUGME_W);
	lv_obj_t *rk = kb_satir(b, "Renk");
	for (int i = 0; i < KY_RENK_N; i++) {
		lv_obj_t *c = lv_button_create(rk);
		lv_obj_set_size(c, i ? 56 : KB_DUGME_W, 50);
		lv_obj_set_style_radius(c, 14, 0);
		lv_obj_set_style_shadow_width(c, 0, 0);
		lv_obj_set_style_bg_color(c, i ? lv_color_hex(ky_renk_govde[i]) : C_ACC, 0);
		if (!i) {
			lv_obj_t *l = label(c, f_m, "Tema");
			lv_obj_center(l);
			lv_obj_set_style_text_color(l, lv_color_white(), 0);
		}
		lv_obj_add_event_cb(c, kb_sec_cb, LV_EVENT_CLICKED, (void *)(uintptr_t)(3 << 8 | i));
		kb_renk[i] = c;
	}
	lv_obj_t *kk = kb_satir(b, "Karakter");
	for (int i = 0; i < KY_KAR_N; i++) {
		kb_kar[i] = dugme(kk, ky_kar_ad[i], KB_DUGME_W + 20, 50, kb_sec_cb, (void *)(uintptr_t)(4 << 8 | i), 0);	/* 0.68 */
		lv_obj_set_style_pad_hor(kb_kar[i], 4, 0);
	}
	lv_obj_t *bs2 = duz(kk, 16, 1);
	(void)bs2;
	dugme(kk, LV_SYMBOL_PLAY "  Önizle", 160, 50, kb_onizle_cb, NULL, 1);
	kb_esitle();
}

static lv_obj_t *ay_sw_ters;	/* 0.47 */

static void ters_ayarla(int on)
{
	on = !!on;
	if (on == ekran_ters)
		return;
	ekran_ters = on;
	if (on)
		unlink("/data/panel-ekran-duz");
	else {
		int fd = open("/data/panel-ekran-duz", O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
		if (fd >= 0)
			close(fd);
	}
	sync();
	if (ay_sw_ters)
		sw_ayarla(ay_sw_ters, on);
	lv_obj_invalidate(lv_screen_active());
	lv_obj_invalidate(lv_layer_top());
	printf("ekran 180 derece: %s\n", on ? "acik" : "kapali");
}

static void ters_sw_cb(lv_event_t *e)
{
	ters_ayarla(lv_obj_has_state(lv_event_get_target(e), LV_STATE_CHECKED));
}

static void gc_sw_cb(lv_event_t *e)	/* 0.33 gecis animasyonlari */
{
	gc_acik = lv_obj_has_state(lv_event_get_target(e), LV_STATE_CHECKED);
	if (gc_acik)
		unlink("/data/panel-gecis-kapali");
	else {
		int fd = open("/data/panel-gecis-kapali", O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
		if (fd >= 0)
			close(fd);
	}
	sync();
}

/* 0.53 YENIDEN BASLAT (onayli) / ASISTANI YENILE. Panel kok yetkisiyle calisir. Yeniden baslatmadan once Wi-Fi guc
 * korumasi isareti silinir (Merkez v9 ile ayni). Test: PANEL_YENIDEN_TEST / PANEL_ASISTAN_YENILE_TEST dosya yolu verilirse
 * reboot / kill yerine o dosyaya yazilir. */
static lv_obj_t *yb_mb;

static void yb_mb_kapat(void)
{
	if (yb_mb) {
		lv_msgbox_close(yb_mb);
		yb_mb = NULL;
	}
}

static void yb_simdi_cb(lv_timer_t *t)
{
	(void)t;
	const char *test = getenv("PANEL_YENIDEN_TEST");
	if (test) {
		FILE *f = fopen(test, "w");
		if (f) {
			fputs("yeniden-baslat\n", f);
			fclose(f);
		}
		printf("yeniden baslatma (test): %s\n", test);
		return;
	}
	unlink("/data/.wifi-deneme");
	sync();
	printf("yeniden baslatma: reboot(RB_AUTOBOOT)\n");
	fflush(stdout);
	reboot(RB_AUTOBOOT);
	printf("yeniden baslatma basarisiz: %s\n", strerror(errno));
}

static void yb_onay_cb(lv_event_t *e)
{
	int evet = (int)(uintptr_t)lv_event_get_user_data(e);
	yb_mb_kapat();
	if (!evet) {
		printf("yeniden baslatma: vazgecildi\n");
		return;
	}
	printf("yeniden baslatma: panelden onaylandi\n");
	show_toast(LV_SYMBOL_POWER "  Yeniden başlatılıyor… Wi-Fi yaklaşık 1 dakikada döner");
	lv_timer_t *t = lv_timer_create(yb_simdi_cb, 1500, NULL);
	lv_timer_set_repeat_count(t, 1);
}

static void yb_mb_btn(lv_obj_t *mb, const char *t, int evet)
{
	lv_obj_t *b = lv_msgbox_add_footer_button(mb, t);
	lv_obj_set_height(b, 64);
	lv_obj_set_width(b, 260);
	lv_obj_set_style_radius(b, 14, 0);
	lv_obj_set_style_text_font(b, f_mb, 0);
	lv_obj_set_style_bg_color(b, evet ? lv_color_hex(0xDC2626) : lv_color_hex(TH.btn), 0);
	lv_obj_set_style_text_color(b, evet ? lv_color_white() : lv_color_hex(TH.text), 0);
	lv_obj_add_event_cb(b, yb_onay_cb, LV_EVENT_CLICKED, (void *)(uintptr_t)evet);
}

static void ay_yeniden_cb(lv_event_t *e)
{
	(void)e;
	if (yb_mb)
		return;
	yb_mb = lv_msgbox_create(NULL);
	lv_obj_set_width(yb_mb, 900);
	lv_obj_set_style_radius(yb_mb, 20, 0);
	lv_obj_set_style_bg_color(yb_mb, C_CARD, 0);
	lv_obj_set_style_border_width(yb_mb, 1, 0);
	lv_obj_set_style_border_color(yb_mb, lv_color_hex(TH.sep), 0);
	lv_obj_set_style_shadow_width(yb_mb, 0, 0);
	lv_obj_set_style_text_color(yb_mb, lv_color_hex(TH.text), 0);
	lv_obj_set_style_pad_all(yb_mb, 24, 0);
	lv_obj_set_style_pad_row(yb_mb, 14, 0);
	lv_obj_t *t = lv_msgbox_add_title(yb_mb, LV_SYMBOL_POWER "  Cihaz yeniden başlatılsın mı?");
	lv_obj_set_style_text_font(t, f_l, 0);
	lv_obj_t *h = lv_msgbox_get_header(yb_mb);
	if (h) {
		lv_obj_set_style_bg_opa(h, LV_OPA_TRANSP, 0);
		lv_obj_set_height(h, LV_SIZE_CONTENT);
	}
	lv_obj_t *x = lv_msgbox_add_text(yb_mb, "Panel, Hey Köle, kamera ve Wi-Fi kapanıp yeniden açılır (yaklaşık 1 dakika). "
					   "Kayıtlar ve ayarlar korunur.\nWi-Fi kendiliğinden bağlanmazsa: Ayarlar > Bağlantılar > Wi-Fi ağı.");
	lv_obj_set_style_text_font(x, f_m, 0);
	lv_obj_set_style_text_line_space(x, 6, 0);
	lv_obj_set_style_text_color(x, C_DIM, 0);
	yb_mb_btn(yb_mb, "Vazgeç", 0);
	yb_mb_btn(yb_mb, LV_SYMBOL_POWER "  Yeniden başlat", 1);
	lv_obj_t *ft = lv_msgbox_get_footer(yb_mb);
	if (ft) {
		lv_obj_set_height(ft, LV_SIZE_CONTENT);
		lv_obj_set_style_bg_opa(ft, LV_OPA_TRANSP, 0);
		lv_obj_set_style_pad_top(ft, 10, 0);
		lv_obj_set_style_pad_column(ft, 16, 0);
	}
	printf("yeniden baslatma: onay penceresi\n");
}

/* calisan kole-asistan-vN surecleri (comm) -> SIGTERM; autorun dongusu 5 sn sonra (yeni dosyayla) baslatir */
static int asistan_durdur(void)
{
	DIR *dd = opendir("/proc");
	struct dirent *de;
	int n = 0;
	if (!dd)
		return 0;
	while ((de = readdir(dd))) {
		if (!isdigit((unsigned char)de->d_name[0]))
			continue;
		char y[300], c[32] = "";
		snprintf(y, sizeof y, "/proc/%s/comm", de->d_name);
		FILE *f = fopen(y, "r");
		if (!f)
			continue;
		if (fgets(c, sizeof c, f) && !strncmp(c, "kole-asistan-v", 14) && !kill((pid_t)atoi(de->d_name), SIGTERM))
			n++;
		fclose(f);
	}
	closedir(dd);
	return n;
}

static void ay_asistan_yenile_cb(lv_event_t *e)
{
	(void)e;
	const char *test = getenv("PANEL_ASISTAN_YENILE_TEST");
	int n;
	if (test) {
		FILE *f = fopen(test, "w");
		if (f) {
			fputs("asistan-yenile\n", f);
			fclose(f);
		}
		n = 1;
	} else {
		n = asistan_durdur();
	}
	printf("asistan yenile: %d surec durduruldu%s\n", n, n ? ", autorun 5 sn sonra baslatir" : "");
	show_toast(n ? LV_SYMBOL_REFRESH "  Hey Köle yeniden başlatılıyor (yaklaşık 5 sn)"
		     : LV_SYMBOL_WARNING "  Çalışan asistan bulunamadı");
}

static void ayarlar_build(lv_obj_t *t)
{
	static const char *const nav_ikon[AY_N] = {LV_SYMBOL_IMAGE, LV_SYMBOL_LOOP, LV_SYMBOL_VOLUME_MAX, LV_SYMBOL_AUDIO,
						   LV_SYMBOL_WIFI, LV_SYMBOL_EYE_CLOSE, LV_SYMBOL_SETTINGS};
	static const char *const nav_ad[AY_N] = {"Ekran", "Koruyucu", "Ses", "Köle", "Bağlantılar", "Gizlilik", "Sistem"};	/* 0.54: 7 */
	lv_obj_set_flex_flow(t, LV_FLEX_FLOW_ROW);
	lv_obj_set_style_pad_column(t, 14, 0);
	lv_obj_remove_flag(t, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_t *nav = card(t, NULL);
	lv_obj_set_flex_grow(nav, 0);
	lv_obj_set_width(nav, 300);
	lv_obj_set_style_pad_all(nav, 12, 0);
	lv_obj_set_style_pad_row(nav, 6, 0);	/* 0.54: 7 bolum */
	for (int i = 0; i < AY_N; i++) {
		lv_obj_t *b = lv_button_create(nav);
		lv_obj_set_size(b, LV_PCT(100), 54);	/* 0.30: 6 bolum (72 -> 62); 0.54: 7 bolum (54) */
		lv_obj_set_style_radius(b, 14, 0);
		lv_obj_set_style_shadow_width(b, 0, 0);
		lv_obj_set_style_pad_left(b, 18, 0);
		lv_obj_t *ik = label(b, f_mb, nav_ikon[i]);	/* 0.53: simgeler farkli genislikte; yazilar ayni hizada */
		lv_obj_set_width(ik, 40);
		lv_obj_set_style_text_align(ik, LV_TEXT_ALIGN_CENTER, 0);
		lv_obj_align(ik, LV_ALIGN_LEFT_MID, 0, 0);
		lv_obj_t *l = label(b, f_mb, nav_ad[i]);
		lv_obj_align(l, LV_ALIGN_LEFT_MID, 54, 0);
		lv_obj_add_event_cb(b, ay_nav_cb, LV_EVENT_CLICKED, (void *)(uintptr_t)i);
		ay_nav[i] = b;
	}
	lv_obj_t *icerik = card(t, NULL);
	lv_obj_set_width(icerik, 0);

	/* EKRAN */
	lv_obj_t *b = ay_bol[AY_EKRAN] = ay_bolum(icerik, LV_SYMBOL_IMAGE, "EKRAN");
	lv_obj_t *r = satir(b, "Parlaklık otomatik");
	sw_auto = anahtar(r, br_auto, br_mode_cb);
	l_br_auto = label(b, f_s, "");
	lv_obj_set_style_text_color(l_br_auto, C_DIM, 0);
	br_box = kutu(b, LV_FLEX_FLOW_ROW, 12);
	lv_obj_set_flex_align(br_box, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	sl_br = kaydirici(br_box, LV_PCT(80), 26, 5, 100, br_manual);
	lv_obj_add_event_cb(sl_br, br_slider_cb, LV_EVENT_VALUE_CHANGED, NULL);
	lv_obj_add_event_cb(sl_br, br_slider_cb, LV_EVENT_RELEASED, NULL);
	l_br_pct = label(br_box, f_l, "");
	lv_obj_set_width(l_br_pct, 130);
	lv_obj_set_style_text_align(l_br_pct, LV_TEXT_ALIGN_RIGHT, 0);
	lv_label_set_text_fmt(l_br_pct, "%%%d", br_manual);
	if (br_auto)
		lv_obj_add_flag(br_box, LV_OBJ_FLAG_HIDDEN);
	lv_obj_t *b_ekran = b;	/* 0.54: koruyucu satirlari ayri bolume */
	b = ay_bol[AY_KORUYUCU] = ay_bolum(icerik, LV_SYMBOL_LOOP, "EKRAN KORUYUCU");
	r = satir(b, "Ekran koruyucu");
	lv_obj_t *ssk = kutu(r, LV_FLEX_FLOW_ROW, 14);	/* 0.40: onizle + anahtar; turler alt satirlarda */
	lv_obj_set_width(ssk, LV_SIZE_CONTENT);
	lv_obj_set_flex_align(ssk, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	dugme(ssk, LV_SYMBOL_EYE_OPEN "  Önizle", 170, 52, ay_ss_onizle_cb, NULL, 0);
	sw_ss = anahtar(ssk, ss_on, ss_sw_cb);
	lv_obj_t *trk = kutu(b, LV_FLEX_FLOW_ROW_WRAP, 10);
	lv_obj_set_style_pad_row(trk, 10, 0);
	for (int i = 0; i < SS_TUR_N; i++)
		ay_tur[i] = dugme(trk, ss_tur_ad[i], 226, 52, ay_tur_cb, (void *)(uintptr_t)i, 0);	/* 0.68: 212 -> 226 */	/* 0.44: 11 tur, iki satir */
	lv_obj_t *b_koruyucu = b;
	b = b_ekran;
	r = satir(b, "Gece kipi");	/* 0.41 */
	lv_obj_t *gck = kutu(r, LV_FLEX_FLOW_ROW, 14);
	lv_obj_set_width(gck, LV_SIZE_CONTENT);
	lv_obj_set_flex_align(gck, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	lv_obj_t *gca = label(gck, f_s, "23:00-07:00 · bildirim yok, kısık ışık");
	lv_obj_set_style_text_color(gca, C_DIM, 0);
	ay_sw_gece = anahtar(gck, gece_ayar, gece_sw_cb);
	b = b_koruyucu;
	r = satir(b, "Fotoğraf çerçevesi");	/* 0.37 */
	lv_obj_t *cfk = kutu(r, LV_FLEX_FLOW_ROW, 14);
	lv_obj_set_width(cfk, LV_SIZE_CONTENT);
	lv_obj_set_flex_align(cfk, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	lv_obj_t *cfa = label(cfk, f_s, "SD kart: fotograf klasörü (JPEG)");
	lv_obj_set_style_text_color(cfa, C_DIM, 0);
	ay_sw_cf = anahtar(cfk, cf_acik_mi(), cf_sw_cb);
	r = satir(b, "Koruyucu süresi");
	lv_obj_t *dkk = kutu(r, LV_FLEX_FLOW_ROW, 10);
	lv_obj_set_width(dkk, LV_SIZE_CONTENT);
	for (int i = 0; i < 4; i++) {
		char t2[16];
		snprintf(t2, sizeof t2, "%d dk", ay_dk_deger[i]);
		ay_dk[i] = dugme(dkk, t2, 110, 52, ay_dk_cb, (void *)(uintptr_t)i, 0);
	}
	b = b_ekran;
	r = satir(b, "Tema");
	lv_obj_t *tk = kutu(r, LV_FLEX_FLOW_ROW, 10);
	lv_obj_set_width(tk, LV_SIZE_CONTENT);
	for (int i = 0; i < TEMA_N; i++) {
		lv_obj_t *tb = lv_button_create(tk);
		lv_obj_set_size(tb, 270, 60);	/* 0.33: 300 -> 240; 0.54: 270 (Kirmizi & Siyah sigsin; anahtarlar alt satirda) */
		lv_obj_set_style_radius(tb, 14, 0);
		lv_obj_set_style_shadow_width(tb, 0, 0);
		lv_obj_set_style_bg_color(tb, lv_color_hex(temalar[i].card), 0);
		lv_obj_set_style_border_width(tb, i == tema ? 4 : 2, 0);
		lv_obj_set_style_border_color(tb, lv_color_hex(temalar[i].acc), 0);
		lv_obj_set_flex_flow(tb, LV_FLEX_FLOW_ROW);
		lv_obj_set_flex_align(tb, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
		lv_obj_set_style_pad_column(tb, 10, 0);
		lv_obj_set_style_pad_left(tb, 14, 0);	/* 0.68: "Kirmizi & Siyah" kenara dayaniyordu */
		lv_obj_set_style_pad_right(tb, 8, 0);
		daire(tb, 26, lv_color_hex(temalar[i].acc));
		lv_obj_t *l = label(tb, f_mb, temalar[i].ad);
		lv_obj_set_style_text_color(l, lv_color_hex(temalar[i].text), 0);
		lv_obj_add_event_cb(tb, tema_cb, LV_EVENT_CLICKED, (void *)(uintptr_t)i);
	}
	r = satir(b, "Animasyon");	/* 0.54: Tema satiri yatayda tasiyordu -> ayri satirlar */
	anahtar(r, gc_acik, gc_sw_cb);
	r = satir(b, "Ekranı 180° çevir");
	ay_sw_ters = anahtar(r, ekran_ters, ters_sw_cb);

	/* SES */
	b = ay_bol[AY_SES] = ay_bolum(icerik, LV_SYMBOL_VOLUME_MAX, "SES");
	r = satir(b, "Ses düzeyi");
	ay_l_ses = label(r, f_l, "");
	ay_sl_ses = kaydirici(b, LV_PCT(100), 30, 0, 100, ses_duzey);
	lv_obj_add_event_cb(ay_sl_ses, ay_ses_cb, LV_EVENT_VALUE_CHANGED, NULL);
	lv_obj_add_event_cb(ay_sl_ses, ay_ses_cb, LV_EVENT_RELEASED, NULL);
	r = satir(b, "Sessiz");
	ay_sw_sessiz = anahtar(r, ses_sessiz, ay_sessiz_cb);
	r = satir(b, "Hoparlör testi");	/* 0.39 */
	dugme(r, LV_SYMBOL_VOLUME_MAX "  Test tonu", 230, 52, ay_sktest_cb, NULL, 1);
	ay_l_codec = soluk(b, f_s, "");
	ay_l_sktest = soluk(b, f_s, "");
	r = satir(b, "Bilgisayarın sesi");
	lv_obj_t *pk = kutu(r, LV_FLEX_FLOW_ROW, 10);
	lv_obj_set_width(pk, LV_SIZE_CONTENT);
	dugme(pk, LV_SYMBOL_VOLUME_MID "  -", 110, 52, md_makro_cb, (void *)"voldown", 0);
	dugme(pk, LV_SYMBOL_MUTE, 90, 52, md_makro_cb, (void *)"mute", 0);
	dugme(pk, LV_SYMBOL_VOLUME_MAX "  +", 110, 52, md_makro_cb, (void *)"volup", 0);

	/* KOLE (0.30) */
	b = ay_bol[AY_KOLE] = ay_bolum(icerik, LV_SYMBOL_AUDIO, "KÖLE");
	kole_bolumu(b);

	/* BAGLANTILAR */
	b = ay_bol[AY_BAGLANTI] = ay_bolum(icerik, LV_SYMBOL_WIFI, "BAĞLANTILAR");
	r = kutu(b, LV_FLEX_FLOW_ROW, 12);
	lv_obj_set_flex_grow(r, 1);
	lv_obj_t *k = ay_alt_kart(r, LV_SYMBOL_WIFI, "Wi-Fi");
	ay_l_wifi = soluk(k, f_s, "");
	lv_obj_set_style_text_line_space(ay_l_wifi, 6, 0);
	lv_obj_set_flex_grow(ay_l_wifi, 1);
	lv_obj_t *wb = wf_btn(k, LV_SYMBOL_WIFI "  Wi-Fi ağı", wf_open_cb);
	lv_obj_set_width(wb, LV_PCT(100));
	k = ay_alt_kart(r, LV_SYMBOL_BLUETOOTH, "Bluetooth");
	l_bt = label(k, f_s, "Bluetooth bilgisi bekleniyor");
	lv_obj_set_width(l_bt, LV_PCT(100));
	lv_label_set_long_mode(l_bt, LV_LABEL_LONG_WRAP);
	lv_obj_set_style_text_line_space(l_bt, 6, 0);
	ay_l_btad = soluk(k, f_s, "");
	lv_obj_set_flex_grow(ay_l_btad, 1);	/* 0.53: dugmeler kartin altinda (Wi-Fi karti gibi) */
	lv_obj_t *btn = soluk(k, f_s, "Kapalıyken bağlantı kesilir, görünmez olur; telefon yeniden bağlanırsa tekrar kesilir.");
	lv_label_set_long_mode(btn, LV_LABEL_LONG_WRAP);
	lv_obj_set_width(btn, LV_PCT(100));
	lv_obj_t *btr = kutu(k, LV_FLEX_FLOW_ROW, 10);	/* 0.55: Bluetooth Acik/Kapali ayari */
	lv_obj_set_flex_align(btr, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	label(btr, f_mb, "Bluetooth açık");
	bt_sw = anahtar(btr, !bt_kapali_mi(), bt_sw_cb);
	k = ay_alt_kart(r, LV_SYMBOL_DRIVE, "Bilgisayar");
	ay_l_pc = soluk(k, f_s, "");
	lv_obj_set_style_text_line_space(ay_l_pc, 6, 0);

	/* GIZLILIK */
	b = ay_bol[AY_GIZLILIK] = ay_bolum(icerik, LV_SYMBOL_EYE_CLOSE, "GİZLİLİK");
	r = satir(b, "Gizlilik kalkanı");
	ay_sw_kalkan = anahtar(r, g_kalkan, gz_sw_cb);
	soluk(b, f_s, "Açıkken kamera her saniye kapatılır, uzaktan açılamaz. Panel yeniden başlayınca kapanır.");
	r = satir(b, "Rahatsız etme");
	ay_sw_dnd = anahtar(r, dnd, ay_dnd_cb);
	soluk(b, f_s, "PC'den gelen bildirimler ekranda gösterilmez, bildirim geçmişine yazılır. Zamanlayıcı uyarısı yine görünür.");
	r = satir(b, "Kamera");
	lv_obj_t *kmk = kutu(r, LV_FLEX_FLOW_ROW, 16);	/* 0.54: Canli goruntu burada da */
	lv_obj_set_width(kmk, LV_SIZE_CONTENT);
	lv_obj_set_flex_align(kmk, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	ay_l_kam = label(kmk, f_m, "-");
	dugme(kmk, LV_SYMBOL_VIDEO "  Arşiv", 180, 56, av_ac_cb, (void *)(uintptr_t)AR_KAYIT, 0);	/* 0.56 */
	dugme(kmk, LV_SYMBOL_EYE_OPEN "  Canlı görüntü", 290, 56, kg_ac_cb, NULL, 1);
	r = satir(b, "Time-lapse");	/* 0.55 */
	lv_obj_t *zlk = kutu(r, LV_FLEX_FLOW_ROW, 16);
	lv_obj_set_width(zlk, LV_SIZE_CONTENT);
	lv_obj_set_flex_align(zlk, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	zl_l = label(zlk, f_s, "");
	lv_obj_set_style_text_color(zl_l, C_DIM, 0);
	zl_sw = anahtar(zlk, zl_acik(), zl_sw_cb);
	r = satir(b, "Kare aralığı");
	lv_obj_t *zla = kutu(r, LV_FLEX_FLOW_ROW, 8);
	lv_obj_set_width(zla, LV_SIZE_CONTENT);
	for (int i = 0; i < ZL_N; i++)	/* 0.74: 1 sn, 3 sn eklendi */
		zl_b[i] = dugme(zla, zl_aralik_ad[i], 104, 52, zl_aralik_cb, (void *)(uintptr_t)i, 0);
	r = satir(b, "Olay tespiti");	/* 0.74 */
	zl_olay_sw = anahtar(r, zl_olay_acik(), zl_olay_sw_cb);
	soluk(b, f_s, "Time-lapse sırasında hareket izlenir: dakikalık hareket puanı olay şeridine yazılır, hareket anları ek "
		      "kare olarak eklenir ve Telegram'a fotoğraf gider. Olaylar Merkez ve PC Merkez'de atlanabilir.");
	zl_dl = soluk(b, f_s, "");
	zl_esitle();
	lv_obj_t *gb = dugme(b, LV_SYMBOL_EYE_CLOSE "  Gizlilik sayfasını aç", LV_PCT(100), 60, ay_git_cb,
			     (void *)(uintptr_t)S_GIZLILIK, 0);
	(void)gb;

	/* SISTEM / HAKKINDA */
	b = ay_bol[AY_SISTEM] = ay_bolum(icerik, LV_SYMBOL_SETTINGS, "SİSTEM & HAKKINDA");
	r = kutu(b, LV_FLEX_FLOW_ROW, 18);
	lv_obj_set_flex_grow(r, 1);
	lv_obj_t *sol = kutu(r, LV_FLEX_FLOW_ROW, 16);
	lv_obj_set_size(sol, 0, LV_PCT(100));
	lv_obj_set_flex_grow(sol, 3);	/* 0.42: sol 3 / hizmetler 4 / bildirimler 2 */
	lv_obj_t *slogo = lv_image_create(sol);
	lv_image_set_src(slogo, &logo_tam);
	lv_image_set_scale(slogo, 80);	/* 0.42: hizmetler sutununa yer */
	lv_obj_set_size(slogo, logo_tam.header.w * 80 / 256, logo_tam.header.h * 80 / 256);
	lv_image_set_inner_align(slogo, LV_IMAGE_ALIGN_CENTER);
	l_sys = label(sol, f_s, "");
	lv_obj_set_width(l_sys, 0);
	lv_obj_set_flex_grow(l_sys, 1);
	lv_label_set_long_mode(l_sys, LV_LABEL_LONG_WRAP);
	lv_obj_set_style_text_line_space(l_sys, 5, 0);
	lv_obj_t *hzk = kutu(r, LV_FLEX_FLOW_COLUMN, 6);	/* 0.42 */
	lv_obj_set_size(hzk, 0, LV_PCT(100));
	lv_obj_set_flex_grow(hzk, 5);	/* 0.53: hizmet adlari kesiliyordu */
	baslik_satiri(hzk, LV_SYMBOL_LIST, "HİZMETLER");
	lv_obj_t *hzg = kutu(hzk, LV_FLEX_FLOW_ROW_WRAP, 6);
	lv_obj_set_style_pad_column(hzg, 12, 0);
	for (int i = 0; i < HZ_N; i++) {
		lv_obj_t *hr = kutu(hzg, LV_FLEX_FLOW_ROW, 8);
		lv_obj_set_width(hr, LV_PCT(48));
		lv_obj_set_flex_align(hr, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
		hz_nokta[i] = daire(hr, 12, C_DIM);
		hz_l[i] = label(hr, f_s, hz_tanim[i].ad);
		lv_obj_set_width(hz_l[i], 0);
		lv_obj_set_flex_grow(hz_l[i], 1);
		lv_label_set_long_mode(hz_l[i], LV_LABEL_LONG_DOT);
	}
	hz_l_ozet = soluk(hzk, f_s, "");
	lv_obj_set_style_text_line_space(hz_l_ozet, 4, 0);
	hz_son = 0;
	lv_obj_t *sg = kutu(r, LV_FLEX_FLOW_COLUMN, 8);
	lv_obj_set_size(sg, 0, LV_PCT(100));
	lv_obj_set_flex_grow(sg, 2);
	baslik_satiri(sg, LV_SYMBOL_BELL, "SON BİLDİRİMLER");
	ay_l_bld = soluk(sg, f_s, "");
	lv_obj_set_style_text_line_space(ay_l_bld, 6, 0);
	r = kutu(b, LV_FLEX_FLOW_ROW, 12);
	dugme(r, LV_SYMBOL_EYE_CLOSE "  Kilit", 180, 60, ay_git_cb, (void *)(uintptr_t)100, 0);	/* 0.53: 5 dugme */
	dugme(r, LV_SYMBOL_IMAGE "  Koruyucu", 214, 60, ay_git_cb, (void *)(uintptr_t)101, 0);
	dugme(r, LV_SYMBOL_DOWN "  Denetim", 204, 60, ay_git_cb, (void *)(uintptr_t)102, 0);
	dugme(r, LV_SYMBOL_REFRESH "  Asistanı yenile", 270, 60, ay_asistan_yenile_cb, NULL, 0);
	dugme(r, LV_SYMBOL_POWER "  Yeniden başlat", 270, 60, ay_yeniden_cb, NULL, 1);

	ayar_bolum_sec(ay_secili);
}

/* Ayarlar acikken saniyede bir: degisken metinler */
static void ayarlar_tick(void)
{
	char b[400], s[128], yol[160], ad[64] = "";
	if (!ay_l_wifi)
		return;
	ses_kart_tick();	/* 0.39 */
	if (ay_secili == AY_GIZLILIK)
		zl_esitle();	/* 0.55 */
	if (ay_secili == AY_SISTEM && (!hz_son || tick_ms() - hz_son >= 5000)) {	/* 0.42 */
		hz_son = tick_ms();
		hz_guncelle();
	}
	int bagli = strcmp(g_ip, "-") != 0;
	if (bagli) {
		if (wifi_duzey(g_dbm))
			snprintf(s, sizeof s, "%d dBm (%d/4)", g_dbm, wifi_duzey(g_dbm));
		else
			snprintf(s, sizeof s, "-");
		snprintf(b, sizeof b, "Bağlı: %s\nIP: %s\nSinyal: %s", wl_ssid, g_ip, s);
	} else
		snprintf(b, sizeof b, "Bağlı değil.\nAşağıdaki düğmeyle ağ seç ve şifre gir (kaydedilir).");
	gz_metin(ay_l_wifi, b);
	snprintf(b, sizeof b, "%s\nAdres: %s\nPort: %d (TCP, satır başına JSON)\nPC tarafı: KÖLE · PC servisi (tepside, Wi-Fi)",
		 g_pc_ok ? "PC ajanı bağlı" : "PC ajanı bağlı değil", pc_peer, PORT);
	gz_metin(ay_l_pc, b);
	veri_yol(yol, sizeof yol, "bt-ad");
	if (dosya_oku(yol, ad, sizeof ad) > 0)
		ad[strcspn(ad, "\r\n")] = 0;
	snprintf(b, sizeof b, "Kayıtlı ad: %s", ad[0] ? ad : "VeLinux KÖLE");
	gz_metin(ay_l_btad, b);
	gz_metin(ay_l_kam, g_kam || g_kayit ? "AÇIK" : "kapalı");
	gz_renk(ay_l_kam, g_kam || g_kayit ? C_BAD : C_OK);
	b[0] = 0;
	for (int i = 0; i < 6; i++) {
		if (bildirim_al(i, s, sizeof s))
			break;
		snprintf(b + strlen(b), sizeof b - strlen(b), "%s%s", i ? "\n" : "", s);
	}
	gz_metin(ay_l_bld, b[0] ? b : "Henüz bildirim yok.");
}

/* tum anahtar/kaydirici/etiketleri durum degiskenlerinden esitle (Denetim Merkezi <-> Ayarlar <-> Gizlilik) */
static void ayar_esitle(void)
{
	char b[32];
	sw_ayarla(sw_auto, br_auto);
	gizle(br_box, br_auto);
	if (sl_br && !lv_slider_is_dragged(sl_br) && lv_slider_get_value(sl_br) != br_manual)
		lv_slider_set_value(sl_br, br_manual, LV_ANIM_OFF);
	if (l_br_pct) {
		snprintf(b, sizeof b, "%%%d", br_manual);
		gz_metin(l_br_pct, b);
	}
	sw_ayarla(sw_ss, ss_on);
	for (int i = 0; i < SS_TUR_N; i++)	/* 0.24 koruyucu turu */
		if (ay_tur[i]) {
			bg_renk(ay_tur[i], i == ss_tur ? C_ACC : lv_color_hex(TH.btn));
			gz_renk(lv_obj_get_child(ay_tur[i], 0), i == ss_tur ? lv_color_white() : lv_color_hex(TH.text));
		}
	for (int i = 0; i < 4; i++)
		if (ay_dk[i]) {
			int sec = ay_dk_deger[i] == ss_dk;
			bg_renk(ay_dk[i], sec ? C_ACC : lv_color_hex(TH.btn));
			gz_renk(lv_obj_get_child(ay_dk[i], 0), sec ? lv_color_white() : lv_color_hex(TH.text));
		}
	if (ay_sl_ses && !lv_slider_is_dragged(ay_sl_ses) && lv_slider_get_value(ay_sl_ses) != ses_duzey)
		lv_slider_set_value(ay_sl_ses, ses_duzey, LV_ANIM_OFF);
	if (ay_l_ses) {
		snprintf(b, sizeof b, ses_sessiz ? "Sessiz" : "%%%d", ses_duzey);
		gz_metin(ay_l_ses, b);
	}
	sw_ayarla(ay_sw_sessiz, ses_sessiz);
	sw_ayarla(gz_sw, g_kalkan);
	sw_ayarla(ay_sw_kalkan, g_kalkan);
	sw_ayarla(ay_sw_dnd, dnd);
	sw_ayarla(ay_sw_gece, gece_ayar);	/* 0.41 */
	dm_guncelle();
	br_apply();
}

/* sekme degisince (200 ms'de bir denetlenir; LVGL dugme tiklamasinda VALUE_CHANGED gondermiyor) */
/* ===================================================================== 0.45 */
/* Kayan sekme gostergesi + kademeli kart girisi + dokunma halkasi. Saydamlik animasyonu ve golge yok (C906 yazilim
 * cizimi): yalniz oteleme, renk gecisi ve kucuk alanli nesneler. Hepsi gc_acik ("Animasyon") anahtarina bagli. */
#define SK_W 112
static lv_obj_t *sk_gosterge, *sk_sayfa, *dk_hal;
static uint32_t sk_son = UINT32_MAX;
static int sk_tur = 1, sk_sessiz, sk_kn;
static lv_obj_t *sk_kc[8];
static lv_scrollbar_mode_t sk_sb_mod;
static lv_style_transition_dsc_t sk_tr;
static int32_t dk_x, dk_y;

static void sk_tr_hazirla(void)
{
	static const lv_style_prop_t p[] = {LV_STYLE_BG_COLOR, LV_STYLE_BG_OPA, LV_STYLE_TEXT_COLOR, 0};
	lv_style_transition_dsc_init(&sk_tr, p, lv_anim_path_ease_out, 180, 0, NULL);
}

static void sk_x_cb(void *o, int32_t v)
{
	lv_obj_set_x((lv_obj_t *)o, v);
}

static void sk_gosterge_git(uint32_t i, int anim)
{
	lv_obj_t *bar = tabview ? lv_tabview_get_tab_bar(tabview) : NULL, *b = bar ? lv_obj_get_child(bar, (int32_t)i) : NULL;
	if (!sk_gosterge || !b)
		return;
	lv_obj_update_layout(b);
	lv_area_t a, ab;
	lv_obj_get_coords(b, &a);
	lv_obj_get_coords(bar, &ab);
	int32_t x = a.x1 + (lv_area_get_width(&a) - SK_W) / 2;
	lv_obj_set_y(sk_gosterge, ab.y1);
	lv_anim_delete(sk_gosterge, sk_x_cb);
	if (!anim) {
		lv_obj_set_x(sk_gosterge, x);
		return;
	}
	lv_anim_t an;
	lv_anim_init(&an);
	lv_anim_set_var(&an, sk_gosterge);
	lv_anim_set_exec_cb(&an, sk_x_cb);
	lv_anim_set_values(&an, lv_obj_get_x(sk_gosterge), x);
	lv_anim_set_duration(&an, 260);
	lv_anim_set_path_cb(&an, lv_anim_path_ease_in_out);
	lv_anim_start(&an);
}

/* ui_build: alt cubugun ustune (ekranin cocugu, tiklanmaz) gosterge; ust katmana dokunma halkasi (gizli) */
static void sk_gosterge_kur(lv_obj_t *scr, lv_obj_t *bar)
{
	(void)bar;
	sk_sayfa = NULL;
	sk_kn = 0;
	sk_gosterge = lv_obj_create(scr);
	lv_obj_remove_style_all(sk_gosterge);
	lv_obj_set_size(sk_gosterge, SK_W, 24);
	lv_obj_set_style_bg_opa(sk_gosterge, LV_OPA_COVER, 0);
	lv_obj_set_style_bg_color(sk_gosterge, C_ACC, 0);
	lv_obj_set_style_bg_grad_color(sk_gosterge, C_ACC, 0);
	lv_obj_set_style_bg_main_opa(sk_gosterge, 80, 0);
	lv_obj_set_style_bg_grad_opa(sk_gosterge, 0, 0);
	lv_obj_set_style_bg_grad_dir(sk_gosterge, LV_GRAD_DIR_VER, 0);
	lv_obj_remove_flag(sk_gosterge, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_t *cz = lv_obj_create(sk_gosterge);
	lv_obj_remove_style_all(cz);
	lv_obj_set_size(cz, SK_W, 4);
	lv_obj_set_style_radius(cz, 2, 0);
	lv_obj_set_style_bg_opa(cz, LV_OPA_COVER, 0);
	lv_obj_set_style_bg_color(cz, C_ACC, 0);
	lv_obj_remove_flag(cz, LV_OBJ_FLAG_CLICKABLE);
	sk_son = lv_tabview_get_tab_active(tabview);
	sk_gosterge_git(sk_son, 0);

	dk_hal = lv_obj_create(lv_layer_top());
	lv_obj_remove_style_all(dk_hal);
	lv_obj_set_style_radius(dk_hal, LV_RADIUS_CIRCLE, 0);
	lv_obj_set_style_border_width(dk_hal, 3, 0);
	lv_obj_set_style_border_color(dk_hal, C_ACC, 0);
	lv_obj_remove_flag(dk_hal, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_add_flag(dk_hal, LV_OBJ_FLAG_HIDDEN);	/* gizli olmayan ust katman nesnesi PC dogrudan cizimini durdurur */
}

static void sk_y_cb(void *o, int32_t v)
{
	lv_obj_set_style_translate_y((lv_obj_t *)o, v, 0);
}

static void sk_kademe_bitir(void)	/* suren kademeli girisi hemen bitir (hizli sekme degisimi) */
{
	for (int i = 0; i < sk_kn; i++) {
		lv_anim_delete(sk_kc[i], sk_y_cb);
		lv_obj_set_style_translate_y(sk_kc[i], 0, 0);
	}
	if (sk_sayfa)
		lv_obj_set_scrollbar_mode(sk_sayfa, sk_sb_mod);
	sk_kn = 0;
	sk_sayfa = NULL;
}

static void sk_son_bitti(lv_anim_t *a)
{
	(void)a;
	sk_kn = 0;	/* digerleri zaten 0'da; son cocuk da bitti */
	if (sk_sayfa)
		lv_obj_set_scrollbar_mode(sk_sayfa, sk_sb_mod);
	sk_sayfa = NULL;
	if (gc_olcum) {
		printf("gecis: %u kare / %u ms (yenileme %llu ms, aktarim %llu ms, g2d %llu/%llu us) tur=kademeli\n", gc_kare,
		       tick_ms() - gc_t0, (unsigned long long)(gc_cizim_us / 1000), (unsigned long long)(gc_aktarim_us / 1000),
		       pg_say, pg_say ? pg_us / pg_say : 0ULL);
		gc_olcum = 0;
	}
}

static void sk_gec(uint32_t i, int anim)
{
	if (!tabview || i >= S_N || i == sk_son)
		return;
	int ilk = sk_son == UINT32_MAX;
	sk_son = i;
	anim = anim && gc_acik && !ilk && !sk_sessiz;
	sk_gosterge_git(i, anim);
	if (!anim)
		return;
	lv_obj_t *icerik = lv_tabview_get_content(tabview);
	lv_obj_t *sayfa = icerik ? lv_obj_get_child(icerik, (int32_t)i) : NULL;
	if (!sayfa)
		return;
	sk_kademe_bitir();
	gc_kare = 0;
	gc_aktarim_us = gc_cizim_us = 0;
	gc_t0 = tick_ms();
	gc_olcum = 1;
	lv_anim_t a;
	if (!sk_tur) {	/* 0.33 eski kayma (karsilastirma): sayfa 60 px'ten */
		lv_anim_delete(sayfa, gc_anim_cb);
		lv_anim_init(&a);
		lv_anim_set_var(&a, sayfa);
		lv_anim_set_exec_cb(&a, gc_anim_cb);
		lv_anim_set_values(&a, 60, 0);
		lv_anim_set_duration(&a, 220);
		lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
		lv_anim_set_completed_cb(&a, gc_anim_bitti);
		lv_anim_start(&a);
		return;
	}
	uint32_t n = lv_obj_get_child_count(sayfa);
	for (uint32_t c = 0; c < n && sk_kn < 8; c++) {
		lv_obj_t *o = lv_obj_get_child(sayfa, (int32_t)c);
		if (!lv_obj_has_flag(o, LV_OBJ_FLAG_HIDDEN))
			sk_kc[sk_kn++] = o;
	}
	if (!sk_kn) {
		gc_olcum = 0;
		return;
	}
	sk_sayfa = sayfa;
	sk_sb_mod = lv_obj_get_scrollbar_mode(sayfa);
	lv_obj_set_scrollbar_mode(sayfa, LV_SCROLLBAR_MODE_OFF);	/* oteleme kaydirma alanini buyutur: cubuk yanip sonmesin */
	for (int k = 0; k < sk_kn; k++) {
		lv_anim_init(&a);
		lv_anim_set_var(&a, sk_kc[k]);
		lv_anim_set_exec_cb(&a, sk_y_cb);
		lv_anim_set_values(&a, 28, 0);
		lv_anim_set_duration(&a, 300);
		lv_anim_set_delay(&a, (uint32_t)(45 * k));
		lv_anim_set_early_apply(&a, true);
		lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
		if (k == sk_kn - 1)
			lv_anim_set_completed_cb(&a, sk_son_bitti);
		lv_anim_start(&a);
	}
}

static void sekme_ayarla(lv_obj_t *tv, uint32_t i, lv_anim_enable_t an)
{
	(lv_tabview_set_active)(tv, i, an);	/* parantez: makro acilmaz, LVGL'in kendisi */
	if (tv == tabview)
		sk_gec(lv_tabview_get_tab_active(tv), 1);
}

static void dk_anim_cb(void *o, int32_t v)
{
	lv_obj_t *h = (lv_obj_t *)o;
	int32_t s = 28 + 104 * v / 1024;
	lv_obj_set_size(h, s, s);
	lv_obj_set_pos(h, dk_x - s / 2, dk_y - s / 2);
	lv_obj_set_style_border_opa(h, (lv_opa_t)(210 * (1024 - v) / 1024), 0);
}

static void dk_bitti(lv_anim_t *a)
{
	lv_obj_add_flag((lv_obj_t *)a->var, LV_OBJ_FLAG_HIDDEN);
}

static void dk_baslat(int32_t x, int32_t y)
{
	if (!dk_hal || !gc_acik || pc_full || ss_obj || kilitli || aa_obj)
		return;
	dk_x = x;
	dk_y = y;
	lv_anim_delete(dk_hal, dk_anim_cb);
	lv_obj_remove_flag(dk_hal, LV_OBJ_FLAG_HIDDEN);
	lv_obj_move_foreground(dk_hal);
	lv_anim_t a;
	lv_anim_init(&a);
	lv_anim_set_var(&a, dk_hal);
	lv_anim_set_exec_cb(&a, dk_anim_cb);
	lv_anim_set_values(&a, 0, 1024);
	lv_anim_set_duration(&a, 380);
	lv_anim_set_early_apply(&a, true);
	lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
	lv_anim_set_completed_cb(&a, dk_bitti);
	lv_anim_start(&a);
}

static void sekme_izle(void)
{
	if (!tabview)
		return;
	uint32_t a = lv_tabview_get_tab_active(tabview);
	if (a == son_sekme)
		return;
	son_sekme = a;
	if (a == S_GIZLILIK)
		gz_tick();
	else if (a == S_MONITOR) {
		mon_chart_doldur();
		mon_ciz();
	} else if (a == S_PCSIS)	/* 0.67 */
		pcs_ciz();
	else if (a == S_MEDYA)
		medya_tick();
	else if (a == S_AYARLAR)
		ayarlar_tick();
}

/* yeniden kurulum (tema): eski nesneler silinecek; 0.23 isaretcilerini sifirla */
static uint32_t yk_sekme;
static int yeni_sifirla(void)
{
	int acik = dm_sayfa && dm_acik;
	yk_sekme = tabview ? lv_tabview_get_tab_active(tabview) : 0;
	cop_n = 0;	/* bekleyen copler ekran/ust katmanla birlikte silinecek */
	dm_sayfa = dm_perde = NULL;
	dm_yok_et();
	cop_n = 0;
	kl_obj = kl_saat = kl_tarih = kl_sl = NULL;
	kl_priv.kap = NULL;
	not_win = nt_ta = NULL;
	as_win = as_ta = NULL;	/* 0.79: ust katmanla silindi; yoksa "Soru yaz"/"Yeniden baslat" olu kaliyordu (A20) */
	yb_mb = NULL;
	h_dnd.kap = NULL;
	son_sekme = UINT32_MAX;
	return acik;
}

static void yeni_sonra(int dm_acikti)
{
	if (tabview && yk_sekme < S_N) {
		sk_sessiz = 1;	/* 0.45: gosterge yerine atlar, kart girisi yok */
		lv_tabview_set_active(tabview, yk_sekme, LV_ANIM_OFF);
		sk_sessiz = 0;
	}
	if (kilitli) {
		kilitli = 0;
		kilit_goster();
	} else if (dm_acikti)
		dm_git(1, 0);
	ayar_esitle();
}

static void ui_build(void)
{
	lv_obj_t *scr = lv_screen_active();

	lv_obj_set_style_bg_color(scr, C_BG, 0);
	lv_obj_set_style_text_color(scr, lv_color_hex(TH.text), 0);
	lv_obj_remove_flag(scr, LV_OBJ_FLAG_SCROLLABLE);	/* 0.23: seritten asagi surukleme ekrani kaydirmasin */

	/* ust serit (0.23: asagi surukle / dokun = Denetim Merkezi) */
	lv_obj_t *hdr = hdr_obj = lv_obj_create(scr);
	lv_obj_remove_style_all(hdr);
	lv_obj_set_size(hdr, W, 56);
	lv_obj_set_style_pad_hor(hdr, 24, 0);
	lv_obj_set_flex_flow(hdr, LV_FLEX_FLOW_ROW);
	lv_obj_set_flex_align(hdr, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	lv_obj_remove_flag(hdr, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_add_event_cb(hdr, dm_hdr_cb, LV_EVENT_PRESSED, NULL);
	lv_obj_add_event_cb(hdr, dm_hdr_cb, LV_EVENT_PRESSING, NULL);
	lv_obj_add_event_cb(hdr, dm_hdr_cb, LV_EVENT_RELEASED, NULL);
	lv_obj_add_event_cb(hdr, dm_hdr_cb, LV_EVENT_PRESS_LOST, NULL);
	lv_obj_t *brand = lv_obj_create(hdr);
	lv_obj_remove_style_all(brand);
	lv_obj_set_size(brand, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
	lv_obj_set_flex_flow(brand, LV_FLEX_FLOW_ROW);
	lv_obj_set_flex_align(brand, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	lv_obj_set_style_pad_column(brand, 0, 0);
	lv_obj_t *am = lv_image_create(brand);
	lv_image_set_src(am, &amblem_40);	/* 0.43: KOLE robot usak (ARGB, boyanmaz) */
	lv_obj_set_style_pad_right(am, 12, 0);
	lv_obj_t *ky = lv_image_create(brand);
	lv_image_set_src(ky, &kole_yazi_ust);
	if (!TH.dark) {	/* 0.45: Beyaz tema: beyaz robot/yazi acik zeminde gorunmuyordu -> koyu rozet */
		lv_obj_set_style_bg_opa(brand, LV_OPA_COVER, 0);
		lv_obj_set_style_bg_color(brand, lv_color_hex(TH.text), 0);
		lv_obj_set_style_radius(brand, 14, 0);
		lv_obj_set_style_pad_hor(brand, 12, 0);
		lv_obj_set_style_pad_ver(brand, 3, 0);
	}
	tiklanmaz(brand);
	/* 0.22 sag kume: [gizlilik] [kalkan] [rahatsiz etme] [PC] tarih  Wi-Fi cubuklari + IP */
	lv_obj_t *sag = lv_obj_create(hdr);
	lv_obj_remove_style_all(sag);
	lv_obj_set_size(sag, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
	lv_obj_set_flex_flow(sag, LV_FLEX_FLOW_ROW);
	/* START: icerik genisligi (SIZE_CONTENT) END hizalamada sola tasan cocuklari saymiyor, haplar kirpiliyordu */
	lv_obj_set_flex_align(sag, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	lv_obj_set_style_pad_column(sag, 14, 0);
	lv_obj_remove_flag(sag, LV_OBJ_FLAG_SCROLLABLE);
	h_priv = hap(sag);	/* 0.21 gostergesi, artik dolu kirmizi/turuncu hap; dokununca Gizlilik sekmesi */
	lv_obj_add_flag(h_priv.kap, LV_OBJ_FLAG_HIDDEN);
	h_kalkan = hap(sag);
	lv_obj_add_flag(h_kalkan.kap, LV_OBJ_FLAG_HIDDEN);
	h_dnd = hap(sag);	/* 0.23: rahatsiz etme */
	lv_obj_add_flag(h_dnd.kap, LV_OBJ_FLAG_HIDDEN);
	h_pc = hap(sag);
	hap_ayarla(&h_pc, C_DIM, 0, "PC bekleniyor");
	l_hdr_date = label(sag, f_s, "");
	lv_obj_set_style_pad_hor(l_hdr_date, 6, 0);
	lv_obj_t *wfk = lv_obj_create(sag);	/* Wi-Fi: 4 cubuk + IP */
	lv_obj_remove_style_all(wfk);
	lv_obj_set_size(wfk, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
	lv_obj_set_flex_flow(wfk, LV_FLEX_FLOW_ROW);
	lv_obj_set_flex_align(wfk, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER);
	lv_obj_set_style_pad_column(wfk, 3, 0);
	lv_obj_remove_flag(wfk, LV_OBJ_FLAG_SCROLLABLE);
	for (int i = 0; i < 4; i++) {
		wf_bar[i] = lv_obj_create(wfk);
		lv_obj_remove_style_all(wf_bar[i]);
		lv_obj_set_size(wf_bar[i], 6, 8 + i * 5);
		lv_obj_set_style_radius(wf_bar[i], 2, 0);
		lv_obj_set_style_bg_opa(wf_bar[i], LV_OPA_COVER, 0);
		lv_obj_set_style_bg_color(wf_bar[i], lv_color_hex(TH.sep), 0);
	}
	wf_bar_duzey = -1;
	l_hdr_net = label(wfk, f_s, "-");
	lv_obj_set_style_pad_left(l_hdr_net, 8, 0);
	/* 0.23: seritteki her sey tiklanmaz (surukleme/dokunus seride gitsin); yalniz gizlilik haplari Gizlilik'i acar */
	tiklanmaz(sag);
	lv_obj_add_flag(h_priv.kap, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_set_ext_click_area(h_priv.kap, 10);
	lv_obj_add_event_cb(h_priv.kap, hdr_gizlilik_cb, LV_EVENT_CLICKED, NULL);
	lv_obj_add_flag(h_kalkan.kap, LV_OBJ_FLAG_CLICKABLE);
	lv_obj_add_event_cb(h_kalkan.kap, hdr_gizlilik_cb, LV_EVENT_CLICKED, NULL);
	/* 0.23: asagi cekme ipucu (ortada kucuk tutamac) */
	lv_obj_t *tut = duz(hdr, 72, 5);
	lv_obj_add_flag(tut, LV_OBJ_FLAG_IGNORE_LAYOUT);
	lv_obj_set_style_radius(tut, 3, 0);
	lv_obj_set_style_bg_opa(tut, LV_OPA_COVER, 0);
	lv_obj_set_style_bg_color(tut, lv_color_hex(TH.sep), 0);
	lv_obj_align(tut, LV_ALIGN_TOP_MID, 0, 6);
	lv_obj_t *hsep = lv_obj_create(scr);	/* serit alti ince ayrac */
	lv_obj_remove_style_all(hsep);
	lv_obj_set_size(hsep, W - 48, 1);
	lv_obj_set_pos(hsep, 24, 55);
	lv_obj_set_style_bg_opa(hsep, LV_OPA_COVER, 0);
	lv_obj_set_style_bg_color(hsep, lv_color_hex(TH.sep), 0);
	lv_obj_remove_flag(hsep, LV_OBJ_FLAG_CLICKABLE);

	/* sekmeler (alt). 0.23: 10 sekme, simge ustte + ad altta (iki satir, f_s) */
	static const char *const sekme_etiket[S_N] = {
		LV_SYMBOL_HOME "\nPano", LV_SYMBOL_AUDIO "\nMedya", LV_SYMBOL_KEYBOARD "\nKısayollar",
		LV_SYMBOL_BELL "\nAsistan", LV_SYMBOL_LOOP "\nZamanlayıcı", LV_SYMBOL_EDIT "\nNotlar",
		LV_SYMBOL_CHARGE "\nMonitör", LV_SYMBOL_DRIVE "\nPC Sistem", LV_SYMBOL_IMAGE "\nPC Ekranı", LV_SYMBOL_EYE_CLOSE "\nGizlilik",
		LV_SYMBOL_SETTINGS "\nAyarlar"};
	lv_obj_t *tv = tabview = lv_tabview_create(scr);
	lv_obj_set_size(tv, W, H - 56);
	lv_obj_set_y(tv, 56);
	lv_obj_set_style_bg_color(tv, C_BG, 0);
	lv_tabview_set_tab_bar_position(tv, LV_DIR_BOTTOM);
	lv_tabview_set_tab_bar_size(tv, 70);
	lv_obj_t *bar = lv_tabview_get_tab_bar(tv);
	lv_obj_set_style_text_font(bar, f_s, 0);
	lv_obj_set_style_bg_color(bar, lv_color_hex(TH.bar), 0);
	lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, 0);
	lv_obj_t *tabs[S_N];
	for (int i = 0; i < S_N; i++) {
		tabs[i] = lv_tabview_add_tab(tv, sekme_etiket[i]);
		lv_obj_set_style_pad_all(tabs[i], 12, 0);
		lv_obj_set_style_bg_color(tabs[i], C_BG, 0);
	}
	lv_obj_add_event_cb(tv, sekme_degisti_cb, LV_EVENT_VALUE_CHANGED, NULL);
	/* 0.22 sekme dugmeleri: soluk yazi; secili = vurgu rengi + kart zemini. 0.45: ust cizgi yerine kayan gosterge */
	sk_tr_hazirla();
	for (uint32_t i = 0; i < lv_obj_get_child_count(bar); i++) {
		lv_obj_t *b = lv_obj_get_child(bar, (int32_t)i);
		lv_obj_set_style_text_color(b, C_DIM, 0);
		lv_obj_set_style_text_color(b, C_ACC, LV_STATE_CHECKED);
		lv_obj_set_style_bg_color(b, C_CARD, LV_STATE_CHECKED);
		lv_obj_set_style_bg_opa(b, LV_OPA_COVER, LV_STATE_CHECKED);
		lv_obj_set_style_border_width(b, 0, LV_STATE_CHECKED);
		lv_obj_set_style_bg_color(b, C_CARD, 0);	/* secili degilken de kart rengi (saydam): gecis gri parlamaz */
		lv_obj_set_style_bg_color(b, lv_color_hex(TH.sep), LV_STATE_PRESSED);
		lv_obj_set_style_bg_opa(b, LV_OPA_COVER, LV_STATE_PRESSED);
		lv_obj_set_style_transition(b, &sk_tr, 0);
		lv_obj_set_style_transition(b, &sk_tr, LV_STATE_CHECKED);
		lv_obj_set_style_transition(b, &sk_tr, LV_STATE_PRESSED);
		lv_obj_set_style_pad_top(b, 0, LV_STATE_CHECKED);
		lv_obj_t *l = lv_obj_get_child(b, 0);
		if (l) {
			lv_obj_set_style_text_align(l, LV_TEXT_ALIGN_CENTER, 0);
			lv_obj_set_style_text_line_space(l, -2, 0);
		}
	}
	sk_gosterge_kur(scr, bar);	/* 0.45 */
	lv_obj_t *t1 = tabs[S_PANO], *t2 = tabs[S_KISAYOL], *t3 = tabs[S_ASISTAN], *t5 = tabs[S_ZAMAN],
		 *t6 = tabs[S_PC], *t7 = tabs[S_GIZLILIK];
	lv_obj_remove_flag(t6,LV_OBJ_FLAG_SCROLLABLE);
	l_pc_image=label(t6,f_s,"PC görüntüsü bekleniyor");
	lv_obj_align(l_pc_image,LV_ALIGN_TOP_LEFT,0,2);
	lv_obj_t *pcb=lv_button_create(t6);
	lv_obj_set_size(pcb,300,56);
	lv_obj_align(pcb,LV_ALIGN_TOP_RIGHT,0,0);
	lv_obj_add_event_cb(pcb,pc_full_btn_cb,LV_EVENT_CLICKED,NULL);
	pc_bag_btn = wf_btn(t6, LV_SYMBOL_PLAY "  Bağlan", pc_bag_cb);	/* 0.65: duruma gore Baglan / Baglantiyi kes */
	lv_obj_align(pc_bag_btn, LV_ALIGN_TOP_RIGHT, 0, 64);
	lv_obj_t *pci = wf_btn(t6, LV_SYMBOL_UPLOAD "  PC'den ekran iste", pc_ekran_iste_cb);	/* 0.57 */
	lv_obj_align(pci, LV_ALIGN_TOP_RIGHT, 0, 128);
	pc_ses_btn = wf_btn(t6, LV_SYMBOL_VOLUME_MAX "  Ses: —", pc_ses_cb);	/* 0.58 */
	lv_obj_align(pc_ses_btn, LV_ALIGN_TOP_RIGHT, 0, 192);
	pc_ses_ciz();
	pc_kip_btn = wf_btn(t6, LV_SYMBOL_REFRESH "  Bağlanma: Elle", pc_kip_cb);	/* 0.65 */
	lv_obj_align(pc_kip_btn, LV_ALIGN_TOP_RIGHT, 0, 256);
	pc_kip_ciz();
	pc_bag_ciz();
	lv_obj_center(label(pcb,f_mb,LV_SYMBOL_IMAGE "  Tam ekran"));
	/* onizleme: 1600x600 kaynak 800x300'e olcekli (LVGL donusumu, en fazla 4 Hz yenilenir) */
	pc_image=lv_image_create(t6);
	lv_obj_set_size(pc_image,800,300);
	lv_image_set_inner_align(pc_image,LV_IMAGE_ALIGN_STRETCH);
	lv_obj_align(pc_image,LV_ALIGN_BOTTOM_MID,0,-4);
	if (pc_shm.h) lv_image_set_src(pc_image,&pc_dsc);
	else lv_obj_add_flag(pc_image,LV_OBJ_FLAG_HIDDEN);

	gz_build(t7);

	/* Pano 0.32: saat | HAVA (cihaz kendisi ceker, PC'siz) | PIYASA + DURUM */
	lv_obj_set_flex_flow(t1, LV_FLEX_FLOW_ROW);
	lv_obj_set_style_pad_column(t1, 14, 0);
	lv_obj_remove_flag(t1, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_t *c1 = card(t1, NULL);
	lv_obj_set_flex_grow(c1, 4);
	lv_obj_set_flex_align(c1, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	l_selam = label(c1, f_s, "");	/* 0.23: gunun saatine gore selam */
	lv_obj_set_style_text_color(l_selam, C_DIM, 0);
	lv_obj_set_style_text_letter_space(l_selam, 3, 0);
	lv_obj_t *crow = lv_obj_create(c1);
	lv_obj_remove_style_all(crow);
	lv_obj_set_size(crow, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
	lv_obj_set_flex_flow(crow, LV_FLEX_FLOW_ROW);
	lv_obj_set_flex_align(crow, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_END);
	l_clock = label(crow, f_clock, "--:--");
	l_sec = label(crow, f_l, "");
	lv_obj_set_style_text_color(l_sec, C_DIM, 0);
	lv_obj_set_style_pad_bottom(l_sec, 26, 0);
	l_date = label(c1, f_m, "");
	lv_label_set_long_mode(l_date, LV_LABEL_LONG_WRAP);
	lv_obj_set_width(l_date, LV_PCT(100));
	lv_obj_set_style_text_align(l_date, LV_TEXT_ALIGN_CENTER, 0);
	pn_namaz = NULL;	/* 0.48: namaz/imsak satiri kaldirildi (kullanıcı); pn_guncelle NULL'da atlar */
	pn_haber = label(c1, f_s, "");
	lv_label_set_long_mode(pn_haber, LV_LABEL_LONG_WRAP);
	lv_obj_set_width(pn_haber, LV_PCT(100));
	lv_obj_set_style_text_align(pn_haber, LV_TEXT_ALIGN_CENTER, 0);
	lv_obj_set_style_text_color(pn_haber, C_DIM, 0);
	lv_obj_set_style_pad_top(pn_haber, 8, 0);

	/* eski PC/takvim/hava etiketleri: apply_state yazmaya devam eder; gorunmez (DURUM karti ozetler) */
	lv_obj_t *gizli = lv_obj_create(t1);
	lv_obj_remove_style_all(gizli);
	lv_obj_add_flag(gizli, LV_OBJ_FLAG_HIDDEN);
	l_weather = label(gizli, f_s, "");
	l_weather2 = label(gizli, f_s, "");
	l_pcname = label(gizli, f_s, "");
	l_cpu = metric(gizli, "İşlemci", &b_cpu);
	l_ram = metric(gizli, "Bellek", &b_ram);
	l_disk = metric(gizli, "Disk", &b_disk);
	l_netio = label(gizli, f_s, "");
	l_pcup = label(gizli, f_s, "");
	l_events = label(gizli, f_s, "");

	/* HAVA */
	lv_obj_t *c2 = card_i(t1, LV_SYMBOL_GPS, "HAVA");
	lv_obj_set_flex_grow(c2, 5);
	lv_obj_set_style_pad_row(c2, 6, 0);
	hv_sehir = label(c2, f_m, "Ankara");
	lv_obj_set_style_text_color(hv_sehir, C_DIM, 0);
	lv_obj_t *hr = kutu(c2, LV_FLEX_FLOW_ROW, 18);
	lv_obj_set_flex_align(hr, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	hv_ikon = lv_obj_create(hr);
	lv_obj_remove_style_all(hv_ikon);
	lv_obj_set_size(hv_ikon, 150, 120);
	lv_obj_add_event_cb(hv_ikon, hv_ciz_cb, LV_EVENT_DRAW_MAIN_END, NULL);
	lv_obj_t *hs = kutu(hr, LV_FLEX_FLOW_COLUMN, 2);
	lv_obj_set_width(hs, LV_SIZE_CONTENT);
	hv_sicak = label(hs, f_xl, "--°");
	hv_durum = label(hs, f_l, "Veri bekleniyor");
	lv_obj_set_style_text_color(hv_durum, lv_color_hex(TH.acc2), 0);
	hv_detay = label(c2, f_m, "");
	hv_yarin = label(c2, f_s, "");
	lv_label_set_long_mode(hv_yarin, LV_LABEL_LONG_WRAP);
	lv_obj_set_width(hv_yarin, LV_PCT(100));
	lv_obj_set_style_text_color(hv_yarin, C_DIM, 0);
	hf_kur(c2);	/* 0.45: 7 gunluk serit (Yarin satiri gizlenir) */
	hv_saat = label(c2, f_s, "");
	lv_obj_set_style_text_color(hv_saat, C_DIM, 0);

	/* PIYASA + DURUM */
	lv_obj_t *pano_sag = kutu(t1, LV_FLEX_FLOW_COLUMN, 14);
	lv_obj_set_size(pano_sag, 0, LV_PCT(100));
	lv_obj_set_flex_grow(pano_sag, 4);
	lv_obj_t *c3 = card_i(pano_sag, LV_SYMBOL_LIST, "PİYASA");
	lv_obj_set_size(c3, LV_PCT(100), 0);
	lv_obj_set_flex_grow(c3, 1);
	l_fx = label(c3, f_m, "Dolar  -\nEuro  -");
	lv_obj_set_style_text_line_space(l_fx, 6, 0);
	lv_obj_t *c4 = card_i(pano_sag, LV_SYMBOL_SETTINGS, "DURUM");
	lv_obj_set_size(c4, LV_PCT(100), 0);
	lv_obj_set_flex_grow(c4, 1);
	pd_l = label(c4, f_m, "");
	lv_obj_set_style_text_line_space(pd_l, 6, 0);
	lv_label_set_long_mode(pd_l, LV_LABEL_LONG_WRAP);	/* 0.33: 0.32 DOT yalniz ilk satiri gosteriyordu */
	lv_obj_set_width(pd_l, LV_PCT(100));
	hv_kod = -1;
	pv_mtime = 0;	/* yeniden kurulumda veriyi hemen yeniden yaz */

	/* Kisayollar */
	lv_obj_set_flex_flow(t2, LV_FLEX_FLOW_ROW_WRAP);
	lv_obj_set_style_pad_gap(t2, 16, 0);
	lv_obj_set_flex_align(t2, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	macro_cont = t2;

	/* Asistan: istem dugmeleri | yanit */
	lv_obj_set_flex_flow(t3, LV_FLEX_FLOW_ROW);
	lv_obj_set_style_pad_column(t3, 14, 0);
	lv_obj_remove_flag(t3, LV_OBJ_FLAG_SCROLLABLE);
	prompt_cont = lv_obj_create(t3);
	lv_obj_remove_style_all(prompt_cont);
	lv_obj_set_size(prompt_cont, 380, LV_PCT(100));
	lv_obj_set_flex_flow(prompt_cont, LV_FLEX_FLOW_COLUMN);
	lv_obj_set_style_pad_row(prompt_cont, 12, 0);
	lv_obj_t *ac = card_i(t3, LV_SYMBOL_BELL, "ASİSTAN");
	lv_obj_add_flag(ac, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_t *asr = kutu(ac, LV_FLEX_FLOW_ROW, 14);	/* 0.69: serbest soru + durum */
	lv_obj_set_flex_align(asr, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
	dugme(asr, LV_SYMBOL_EDIT "  Soru yaz", 230, 56, as_soru_cb, NULL, 1);
	l_asst_state = label(asr, f_s, "Bir istem seç ya da soru yaz. Cihazdaki Gemini yanıtlar (Wi-Fi; PC gerekmez).");
	lv_obj_set_width(l_asst_state, 0);
	lv_obj_set_flex_grow(l_asst_state, 1);
	lv_label_set_long_mode(l_asst_state, LV_LABEL_LONG_WRAP);
	lv_obj_set_style_text_color(l_asst_state, C_DIM, 0);
	l_asst_soru = label(ac, f_s, "");	/* 0.69: sorulan soru */
	lv_obj_set_width(l_asst_soru, LV_PCT(100));
	lv_label_set_long_mode(l_asst_soru, LV_LABEL_LONG_WRAP);
	lv_obj_set_style_text_color(l_asst_soru, C_ACC, 0);
	l_answer = label(ac, f_m, "");
	lv_label_set_long_mode(l_answer, LV_LABEL_LONG_WRAP);
	lv_obj_set_width(l_answer, LV_PCT(100));
	lv_obj_set_style_text_line_space(l_answer, 6, 0);

	tm_build(t5);
	/* 0.23 yeni sayfalar + Ayarlar (eski Sistem sekmesinin yerine) */
	medya_build(tabs[S_MEDYA]);
	notlar_build(tabs[S_NOTLAR]);
	monitor_build(tabs[S_MONITOR]);
	pcs_build(tabs[S_PCSIS]);	/* 0.67 */
	ayarlar_build(tabs[S_AYARLAR]);

	/* bildirim */
	toast = label(lv_layer_top(), f_mb, "");
	lv_obj_set_style_bg_color(toast, lv_color_hex(TH.toast), 0);
	lv_obj_set_style_text_color(toast, lv_color_hex(TH.toast_text), 0);
	lv_obj_set_style_bg_opa(toast, LV_OPA_90, 0);
	lv_obj_set_style_radius(toast, 14, 0);
	lv_obj_set_style_pad_all(toast, 16, 0);
	lv_obj_align(toast, LV_ALIGN_BOTTOM_MID, 0, -90);	/* 0.23: 70 px sekme cubugunun ustu */
	lv_obj_add_flag(toast, LV_OBJ_FLAG_HIDDEN);

	cJSON *m = cJSON_Parse(default_macros), *p = cJSON_Parse(default_prompts);
	build_buttons(macro_cont, m, 0, 360, 190);
	build_buttons(prompt_cont, p, 1, 380, 96);
	cJSON_Delete(m);
	cJSON_Delete(p);
}

/* ----------------------------------------------------------- PC durumunu uygula */
static double num(cJSON *o, const char *k, double d)
{
	cJSON *v = cJSON_GetObjectItem(o, k);
	return cJSON_IsNumber(v) ? v->valuedouble : d;
}

static const char *str(cJSON *o, const char *k, const char *d)
{
	cJSON *v = cJSON_GetObjectItem(o, k);
	return cJSON_IsString(v) ? v->valuestring : d;
}

static void set_metric(lv_obj_t *l, lv_obj_t *b, double v)
{
	if (v < 0) {
		lv_label_set_text(l, "--");
		return;
	}
	lv_label_set_text_fmt(l, "%d%%", (int)(v + 0.5));
	lv_bar_set_value(b, (int)v, LV_ANIM_ON);
	lv_obj_set_style_bg_color(b, v > 85 ? C_BAD : v > 60 ? C_WARN : C_OK, LV_PART_INDICATOR);
}

static void fmt_rate(char *o, size_t n, double bps)
{
	if (bps >= 1e6)
		snprintf(o, n, "%.1f MB/s", bps / 1e6);
	else
		snprintf(o, n, "%.0f KB/s", bps / 1e3);
}

/* PC'den uzaktan arayuz komutu (test ve otomasyon): {"t":"ui","tab":0..3} {"t":"ui","press":"macro:ID"|"ask:ID"} */
static void apply_ui(const char *line)
{
	cJSON *j = cJSON_Parse(line), *o;

	if (!j)
		return;
	/* 0.24: yalniz {"t":"ui","rg":"<davranis>"} koruyucuyu kapatmaz (robot goz davranisini baslatir) */
	if (cJSON_IsString(o = cJSON_GetObjectItem(j, "rg")) && cJSON_GetArraySize(j) <= 2) {
		if (rg_obj && !rg_kapanis) {
			int d = rg_zorla(&rg, o->valuestring, tick_ms());
			if (rg_tmr)
				lv_timer_ready(rg_tmr);
			printf("robot goz: davranis %s%s\n", o->valuestring, d < 0 ? " (bilinmiyor)" : "");
		}
		cJSON_Delete(j);
		return;
	}
	/* uzaktan komut = etkinlik: koruyucu kapanir. 0.72: yalniz durum/tanilama tasiyan ileti (ajanin 2 sn'lik pc_status'u,
	 * gece parlakligi, PC Sistem/medya verisi, gunluk istegi) etkinlik DEGIL; yoksa PC ajani bagliyken koruyucu hic
	 * acilmiyordu (ui_pasif_v2.h) */
	if (ui_pasif(j)) {
		ui_pasif_n++;
	} else {
		if (ss_obj) {
			ss_hide();
			printf("koruyucu: kapandi (uzaktan komut)\n");
		}
		lv_display_trigger_activity(disp);
	}
	if (cJSON_IsString(o = cJSON_GetObjectItem(j, "br_mode")) && sw_auto) {
		cJSON *pc = cJSON_GetObjectItem(j, "br_pct");
		br_auto = !strcmp(o->valuestring, "auto");
		if (cJSON_IsNumber(pc) && pc->valueint >= 5 && pc->valueint <= 100)
			br_manual = pc->valueint;
		if (br_auto) {
			lv_obj_add_state(sw_auto, LV_STATE_CHECKED);
			lv_obj_add_flag(br_box, LV_OBJ_FLAG_HIDDEN);
		} else {
			lv_obj_remove_state(sw_auto, LV_STATE_CHECKED);
			lv_obj_remove_flag(br_box, LV_OBJ_FLAG_HIDDEN);
		}
		lv_slider_set_value(sl_br, br_manual, LV_ANIM_OFF);
		lv_label_set_text_fmt(l_br_pct, "%%%d", br_manual);
		br_save();
		br_apply();
		ayar_esitle();
	}
	if (cJSON_IsNumber(o = cJSON_GetObjectItem(j, "theme")))
		tema_sec(o->valueint);
	if (cJSON_IsTrue(cJSON_GetObjectItem(j, "wifi")))
		wf_open_cb(NULL);
	else if (cJSON_IsFalse(cJSON_GetObjectItem(j, "wifi")))
		wf_close_cb(NULL);
	if (cJSON_IsString(o = cJSON_GetObjectItem(j, "pc_status")))
		snprintf(pc_status, sizeof pc_status, "%s", o->valuestring);
	if (cJSON_IsString(o = cJSON_GetObjectItem(j, "gunluk"))) {	/* 0.49: salt okunur, yalniz bu uc dosya */
		const char *ad = o->valuestring, *yol = !strcmp(ad, "tts") ? "/tmp/tts.log" : !strcmp(ad, "asistan") ? "/tmp/asistan.log"
							  : !strcmp(ad, "panel") ? "/tmp/panel.log" : NULL;
		cJSON *ar = cJSON_GetObjectItem(j, "ara");	/* 0.56 */
		if (!strcmp(ad, "surec"))
			sr_baslat();
		else if (yol)
			gunluk_gonder(ad, yol, cJSON_IsString(ar) ? ar->valuestring : NULL);
	}
	if (cJSON_IsNumber(o = cJSON_GetObjectItem(j, "ekran_ters")) || cJSON_IsBool(o))	/* 0.47 */
		ters_ayarla(cJSON_IsTrue(o) || (cJSON_IsNumber(o) && o->valueint));
	if (cJSON_IsNumber(o = cJSON_GetObjectItem(j, "gecis_tur"))) {	/* 0.45 olcum: 0 eski kayma, 1 kademeli */
		sk_tur = o->valueint ? 1 : 0;
		printf("gecis turu: %s\n", sk_tur ? "kademeli" : "eski kayma");
	}
	/* 0.23: "tab" 0..6 eski (0.22) sekme numarasi -> yeni sira; yeni sayfalar "sayfa" adiyla */
	if (cJSON_IsNumber(o = cJSON_GetObjectItem(j, "tab")) && o->valueint>=0 && o->valueint<7) {
		dm_git(0, 0);
		lv_tabview_set_active(tabview, eski_sekme[o->valueint], LV_ANIM_OFF);
	}
	if (cJSON_IsString(o = cJSON_GetObjectItem(j, "sayfa")))
		for (int i = 0; i < S_N; i++)
			if (!strcmp(o->valuestring, sekme_adi[i])) {
				dm_git(0, 0);
				lv_tabview_set_active(tabview, (uint32_t)i, LV_ANIM_OFF);
			}
	if (cJSON_IsString(o = cJSON_GetObjectItem(j, "ayar")))
		for (int i = 0; i < AY_N; i++)
			if (!strcmp(o->valuestring, ay_adi[i])) {
				dm_git(0, 0);
				lv_tabview_set_active(tabview, S_AYARLAR, LV_ANIM_OFF);
				ayar_bolum_sec(i);
				ayarlar_tick();
			}
	if (cJSON_IsBool(o = cJSON_GetObjectItem(j, "gece"))) {	/* 0.41 */
		gece_ayarla(cJSON_IsTrue(o));
		ayar_esitle();
	}
	if (cJSON_IsBool(o = cJSON_GetObjectItem(j, "dnd")))
		dnd_ayarla(cJSON_IsTrue(o));
	if (cJSON_IsNumber(o = cJSON_GetObjectItem(j, "ses")) && o->valueint >= 0 && o->valueint <= 100) {
		ses_duzey = o->valueint;	/* 0.39: br_save codec'e uygular */
		ses_sessiz = 0;
		br_save();
		ayar_esitle();
	}
	if (cJSON_IsBool(o = cJSON_GetObjectItem(j, "sessiz"))) {	/* 0.73: sesle "sesi kapat / aç" */
		ses_sessiz = cJSON_IsTrue(o);
		br_save();
		ayar_esitle();
	}
	if (cJSON_IsTrue(cJSON_GetObjectItem(j, "ses_test")))	/* 0.39 */
		ses_test_baslat("uzaktan");
	if (cJSON_IsNumber(o = cJSON_GetObjectItem(j, "koruyucu_dk")) && o->valueint >= 1 && o->valueint <= 60) {
		ss_dk = o->valueint;
		br_save();
		ayar_esitle();
	}
	if (cJSON_IsString(o = cJSON_GetObjectItem(j, "not_ekle"))) {
		if (!not_ekle(&notlar, o->valuestring)) {
			notlar_kaydet();
			notlar_ciz();
		}
	}
	if (cJSON_IsTrue(cJSON_GetObjectItem(j, "not_klavye")))
		not_ac();
	else if (cJSON_IsFalse(cJSON_GetObjectItem(j, "not_klavye")))
		not_kapat();
	if (cJSON_IsObject(o = cJSON_GetObjectItem(j, "pc_sistem")) && !pcs_isle(&pcs, o)) {	/* 0.67 */
		pcs_t = tick_ms() | 1;
		if (pcs.n == 1)
			printf("pc sistem: ilk olcum (%d cekirdek, gpu %d)\n", pcs.cek_n, pcs.gpu_var);
	}
	if (cJSON_IsObject(o = cJSON_GetObjectItem(j, "pc_medya"))) {	/* 0.66: PC'de calan medya */
		const cJSON *a;
		a = cJSON_GetObjectItem(o, "baslik");
		snprintf(pcm_baslik, sizeof pcm_baslik, "%s", cJSON_IsString(a) ? a->valuestring : "");
		a = cJSON_GetObjectItem(o, "sanatci");
		snprintf(pcm_sanatci, sizeof pcm_sanatci, "%s", cJSON_IsString(a) ? a->valuestring : "");
		a = cJSON_GetObjectItem(o, "uygulama");
		snprintf(pcm_uyg, sizeof pcm_uyg, "%s", cJSON_IsString(a) ? a->valuestring : "");
		a = cJSON_GetObjectItem(o, "durum");
		snprintf(pcm_durum, sizeof pcm_durum, "%s", cJSON_IsString(a) ? a->valuestring : "yok");
		pcm_t = tick_ms() | 1;
		printf("pc medya: %s %s (%u bayt)\n", pcm_durum, pcm_uyg, (unsigned)strlen(pcm_baslik));
	}
	if (cJSON_IsString(o = cJSON_GetObjectItem(j, "pc_baglan")))	/* 0.65: {"t":"ui","pc_baglan":"oto"|"elle"} */
		pc_kip_ayarla(!strcmp(o->valuestring, "oto"));
	if (cJSON_IsNumber(o = cJSON_GetObjectItem(j, "pc_full"))) {	/* 0.14: {"t":"ui","pc_full":1|0} */
		if (o->valueint) {
			pc_session_connect(&pc_session);
			pc_dismissed = 0;
			pc_full_enter();
		} else
			pc_full_back_cb(NULL);
	}
	if (cJSON_IsNumber(o = cJSON_GetObjectItem(j, "brightness"))) {
		/* PC'nin otomatik (gece/gunduz) degeri; yalniz otomatik modda uygulanir */
		br_pc = o->valueint < 1 ? 1 : o->valueint > 255 ? 255 : o->valueint;
		br_apply();
		printf("PC parlaklik %d (mod %s), ekran %d\n", br_pc, br_auto ? "otomatik" : "manuel", disp_brightness(-1));
	}
	if (cJSON_IsString(o = cJSON_GetObjectItem(j, "ss_tur")) || cJSON_IsNumber(o)) {	/* 0.24: tur (kaydedilir) */
		int t = -1;
		for (int i = 0; i < SS_TUR_N && cJSON_IsString(o); i++)
			if (!strcmp(o->valuestring, ss_tur_kod[i]))
				t = i;
		if (cJSON_IsNumber(o) && o->valueint >= 0 && o->valueint < SS_TUR_N)
			t = o->valueint;
		if (t >= 0 && t != ss_tur) {
			ss_tur = t;
			br_save();
			ayar_esitle();
			printf("koruyucu turu: %s\n", ss_tur_kod[ss_tur]);
		}
	}
	if (cJSON_IsTrue(o = cJSON_GetObjectItem(j, "ss")) || (cJSON_IsNumber(o) && o->valueint == 1)) {
		if (!ss_obj && !pc_full)	/* 0.22: {"t":"ui","ss":1} ekran koruyucuyu hemen ac */
			ss_show();
	} else if (cJSON_IsString(o) && !strcmp(o->valuestring, "cerceve")) {	/* 0.37 */
		if (!ss_obj && !pc_full && !cf_goster())
			show_toast(LV_SYMBOL_IMAGE "  Fotoğraf yok: SD karttaki fotograf klasörüne JPEG koyun");
	} else if (cJSON_IsString(o)) {	/* 0.24: {"t":"ui","ss":"saat|robot|karisik"} o turla ac (kaydetmez) */
		for (int i = 0; i < SS_TUR_N; i++)
			if (!strcmp(o->valuestring, ss_tur_kod[i]) && !ss_obj && !pc_full)
				ss_show_tur(i);
	}
	if (cJSON_IsBool(o = cJSON_GetObjectItem(j, "cerceve"))) {	/* 0.37 {"t":"ui","cerceve":true|false} */
		cf_ayarla(cJSON_IsTrue(o));
		if (ay_sw_cf) {
			if (cf_acik)
				lv_obj_add_state(ay_sw_cf, LV_STATE_CHECKED);
			else
				lv_obj_remove_state(ay_sw_cf, LV_STATE_CHECKED);
		}
	}
	if (cJSON_IsTrue(cJSON_GetObjectItem(j, "kamera_kapat_sor")))
		gz_btn_cb(NULL);	/* 0.22: yalniz onay penceresini acar (kapatma yine dokunmatik onay ister) */
	if (cJSON_IsTrue(o = cJSON_GetObjectItem(j, "kilit")))	/* 0.23 panel kilidi */
		kilit_goster();
	else if (cJSON_IsFalse(o) && kilitli)
		kilit_ac();
	if (cJSON_IsString(o = cJSON_GetObjectItem(j, "bildirim")) && o->valuestring[0]) {	/* 0.57: cihaz Telegram botuna */
		static time_t bk_son;
		time_t sn = mono_s();
		if (!bk_son || sn - bk_son >= 5) {
			bk_son = sn;
			char m[420];
			snprintf(m, sizeof m, "%.400s", o->valuestring);
			bk_yaz(m);
			printf("bildirim kuyruga: %zu bayt\n", strlen(m));
		} else {
			printf("bildirim: hiz siniri (5 sn)\n");
		}
	}
	if (getenv("PANEL_UZAK_TEST")) {	/* 0.56: arsiv yalniz host provasi */
		if (cJSON_IsString(o = cJSON_GetObjectItem(j, "arsiv")))
			av_ac(!strcmp(o->valuestring, "foto") ? AR_FOTO : !strcmp(o->valuestring, "zaman") ? AR_ZAMAN : AR_KAYIT);
		else if (cJSON_IsFalse(o))
			av_kapat("uzak test");
		if (cJSON_IsNumber(o = cJSON_GetObjectItem(j, "arsiv_sec")))
			av_sec_ac(o->valueint);
		if (cJSON_IsNumber(o = cJSON_GetObjectItem(j, "arsiv_konum")) && av_sec >= 0) {
			av_bekle();
			av_ofs = (long)((long long)o->valueint * av_boy / 1000);
			av_istek = 1;
		}
		if (cJSON_IsBool(o = cJSON_GetObjectItem(j, "arsiv_oynat"))) {
			av_oynuyor = cJSON_IsTrue(o);
			av_oynat_yaz();
		}
	}
	if (cJSON_IsBool(o = cJSON_GetObjectItem(j, "kamera_goruntu")) && getenv("PANEL_UZAK_TEST")) {	/* 0.53: yalniz host provasi */
		if (cJSON_IsTrue(o))
			kg_ac_cb(NULL);
		else
			kg_kapat("uzak test");
	}
	if (cJSON_IsTrue(o = cJSON_GetObjectItem(j, "dm")) || (cJSON_IsNumber(o) && o->valueint == 1))
		dm_git(1, 1);	/* 0.23 Denetim Merkezi */
	else if (cJSON_IsFalse(o) || (cJSON_IsNumber(o) && o->valueint == 0))
		dm_git(0, 1);
	if (cJSON_IsNumber(o = cJSON_GetObjectItem(j, "zamanlayici_sn")) && o->valueint >= 1 && o->valueint <= 7200) {	/* 0.52 */
		tm_mode = "Özel";
		tm_total = tm_left = o->valueint;
		tm_run = 1;
		tm_last_ms = tick_ms();
		tm_draw();
		printf("zamanlayici (uzaktan): %d sn\n", o->valueint);
	}
	if (cJSON_IsString(o = cJSON_GetObjectItem(j, "asistan_sor")) && o->valuestring[0] &&
	    strlen(o->valuestring) <= AS_SORU_MAX) {	/* 0.69 */
		dm_git(0, 0);
		lv_tabview_set_active(tabview, S_ASISTAN, LV_ANIM_OFF);
		as_cihazda_sor(o->valuestring);
	}
	if (cJSON_IsTrue(cJSON_GetObjectItem(j, "asistan_klavye"))) {	/* 0.69 */
		lv_tabview_set_active(tabview, S_ASISTAN, LV_ANIM_OFF);
		as_soru_cb(NULL);
	}
	if (cJSON_IsString(o = cJSON_GetObjectItem(j, "arsiv"))) {	/* 0.70: kamera arsivi (dogrulama) */
		const char *v = o->valuestring;
		if (!strcmp(v, "kapat"))
			av_kapat("uzaktan");
		else if (!strcmp(v, "kayit") || !strcmp(v, "zaman") || !strcmp(v, "foto"))
			av_ac(!strcmp(v, "kayit") ? AR_KAYIT : !strcmp(v, "zaman") ? AR_ZAMAN : AR_FOTO);
	}
	if (cJSON_IsNumber(o = cJSON_GetObjectItem(j, "arsiv_sec")))
		av_sec_ac(o->valueint);
	if (cJSON_IsNumber(o = cJSON_GetObjectItem(j, "arsiv_hiz")) && o->valueint >= 0 && o->valueint < 5) {
		av_hiz = o->valueint;
		av_oynat_yaz();
	}
	if (cJSON_IsString(o = cJSON_GetObjectItem(j, "press"))) {
		const char *v = o->valuestring;
		int kind = !strncmp(v, "ask:", 4);
		const char *id = strchr(v, ':') ? strchr(v, ':') + 1 : v;
		for (int i = 0; i < MAX_BTN; i++)
			if (!strcmp(btn_ids[kind][i], id)) {
				if (kind && (as_gemini_var() || !pc_ajan_var())) {	/* 0.69: once cihazdaki Gemini (0.52: yalniz PC yokken) */
					as_cihazda_sor(btn_lbl[1][i]);
					break;
				}
				char ev[160];
				snprintf(ev, sizeof ev, "{\"t\":\"%s\",\"id\":\"%s\"}", kind ? "ask" : "macro", id);
				send_event(ev);
				if (kind)
					lv_label_set_text(l_asst_state, "Yanıt bekleniyor...");
				else
					show_toast(LV_SYMBOL_OK "  Gönderildi (uzaktan)");
				break;
			}
	}
	cJSON_Delete(j);
}

/* ------------------------------------------------------- 0.32 Pano verisi (PC'siz) + hava simgesi */
static int pv_taze(void)
{
	return pv_zaman && time(NULL) - pv_zaman < 3600;
}

/* 0.45: hava karti (gradyanli zemin: ay kesigi icin o yuksekligin zemin rengi) */
static lv_obj_t *hv_kart;

static lv_color_t hv_zemin(int32_t y)
{
	if (!hv_kart)
		return C_CARD;
	lv_area_t a;
	lv_obj_get_coords(hv_kart, &a);
	lv_color_t ust = lv_obj_get_style_bg_color(hv_kart, 0), alt = lv_obj_get_style_bg_grad_color(hv_kart, 0);
	int32_t dur = (a.y2 - a.y1) * lv_obj_get_style_bg_grad_stop(hv_kart, 0) / 255, dy = y - a.y1;
	if (dur <= 0 || dy >= dur)
		return alt;
	if (dy < 0)
		dy = 0;
	return lv_color_mix(ust, alt, (uint8_t)(255 * (dur - dy) / dur));
}

/* WMO kodu -> simge: 0 gunes/ay, 1-2 gunes+bulut, 3 bulut, 45/48 sis, yagmur, kar, firtina.
 * 0.45: olcekli (o/1024; 1024 = Pano buyuk simgesi), zemin = ay kesigi rengi. */
static void hv_simge(lv_layer_t *l, int32_t cx, int32_t cy, int k, int gece, int32_t o, lv_color_t zemin)
{
#define HS(v) ((int32_t)(v) * o / 1024)
#define HW(v) (HS(v) < 2 ? 2 : HS(v))
	if (k < 0)
		return;
	lv_color_t sari = lv_color_hex(0xFBBF24),	/* acik temada bulut/sis koyu (beyaz zeminde kayboluyordu) */
		   bulut = lv_color_hex(k == 3 || k >= 61 ? (TH.dark ? 0x94A3B8 : 0x64748B) : (TH.dark ? 0xE2E8F0 : 0x94A3B8));
	int bulutlu = k != 0 && !(k == 45 || k == 48);
	if (k <= 2) {	/* gunes ya da ay (bulutluysa sol ustte kucuk) */
		int32_t gx = k ? cx - HS(26) : cx, gy = k ? cy - HS(22) : cy, rr = k ? HS(26) : HS(38);
		if (gece) {
			rg_dortgen(l, gx - rr, gy - rr, gx + rr, gy + rr, LV_RADIUS_CIRCLE, lv_color_hex(0xE5E7EB), LV_OPA_COVER);
			rg_dortgen(l, gx - rr + rr / 2, gy - rr - HS(6), gx + rr + rr / 2, gy + rr - HS(6), LV_RADIUS_CIRCLE, zemin,
				   LV_OPA_COVER);
		} else {
			for (int i = 0; i < 8; i++) {
				float aci = (float)i * 0.785398f;
				ky_cizgi(l, gx + (int32_t)((float)(rr + HS(8)) * cosf(aci)), gy + (int32_t)((float)(rr + HS(8)) * sinf(aci)),
					 gx + (int32_t)((float)(rr + HS(18)) * cosf(aci)), gy + (int32_t)((float)(rr + HS(18)) * sinf(aci)),
					 HW(5), sari);
			}
			rg_dortgen(l, gx - rr, gy - rr, gx + rr, gy + rr, LV_RADIUS_CIRCLE, sari, LV_OPA_COVER);
		}
	}
	if (k == 45 || k == 48) {	/* sis */
		for (int i = 0; i < 4; i++)
			ky_cizgi(l, cx - HS(52) + (i % 2) * HS(14), cy - HS(30) + i * HS(20), cx + HS(52) - (i % 2) * HS(14),
				 cy - HS(30) + i * HS(20), HW(8), bulut);
		return;
	}
	if (bulutlu) {
		int32_t by = k <= 2 ? cy + HS(10) : cy - HS(6);
		rg_dortgen(l, cx - HS(56), by - HS(2), cx + HS(56), by + HS(30), HS(16), bulut, LV_OPA_COVER);
		rg_dortgen(l, cx - HS(46), by - HS(24), cx - HS(2), by + HS(20), LV_RADIUS_CIRCLE, bulut, LV_OPA_COVER);
		rg_dortgen(l, cx - HS(16), by - HS(40), cx + HS(36), by + HS(12), LV_RADIUS_CIRCLE, bulut, LV_OPA_COVER);
	}
	if (k >= 51 && k != 71 && k != 73 && k != 75 && k != 77 && k != 85 && k != 86 && k < 95) {	/* yagmur */
		for (int i = 0; i < 4; i++)
			ky_cizgi(l, cx - HS(36) + i * HS(24), cy + HS(34), cx - HS(44) + i * HS(24), cy + HS(54), HW(5),
				 lv_color_hex(0x38BDF8));
	} else if ((k >= 71 && k <= 77) || k == 85 || k == 86) {	/* kar */
		for (int i = 0; i < 4; i++)
			rg_dortgen(l, cx - HS(42) + i * HS(26), cy + HS(36) + (i % 2) * HS(10), cx - HS(32) + i * HS(26),
				   cy + HS(46) + (i % 2) * HS(10), LV_RADIUS_CIRCLE,
				   TH.dark ? lv_color_white() : lv_color_hex(0x60A5FA), LV_OPA_COVER);
	} else if (k >= 95) {	/* firtina: simsek */
		rg_ucgen(l, cx + HS(4), cy + HS(22), cx - HS(18), cy + HS(50), cx + HS(2), cy + HS(46), sari);
		rg_ucgen(l, cx + HS(2), cy + HS(40), cx + HS(18), cy + HS(40), cx - HS(8), cy + HS(66), sari);
	}
#undef HS
#undef HW
}

static void hv_ciz_cb(lv_event_t *e)
{
	lv_area_t a;
	lv_obj_get_coords(hv_ikon, &a);
	time_t now = time(NULL);
	struct tm tm;
	localtime_r(&now, &tm);
	int32_t cy = (a.y1 + a.y2) / 2;
	hv_simge(lv_event_get_layer(e), (a.x1 + a.x2) / 2, cy, hv_kod, tm.tm_hour < 6 || tm.tm_hour >= 19, 1024, hv_zemin(cy));
}

static const char *pv_para[][2] = {{"USD", "Dolar"}, {"EUR", "Euro"}, {"GBP", "Sterlin"}};

static void pv_tick(lv_timer_t *t)
{
	(void)t;
	struct stat st;
	if (stat("/tmp/panel-veri.json", &st) || st.st_mtime == pv_mtime)
		return;
	pv_mtime = st.st_mtime;
	static char b[4096];	/* 0.45: 2048 -> 4096 (gunler) */
	FILE *f = fopen("/tmp/panel-veri.json", "r");
	if (!f)
		return;
	size_t n = fread(b, 1, sizeof b - 1, f);
	b[n] = 0;
	fclose(f);
	cJSON *j = cJSON_Parse(b), *h, *d;
	if (!j)
		return;
	pv_zaman = (time_t)num(j, "zaman", 0);
	const char *sehir = str(j, "sehir", "");
	if ((h = cJSON_GetObjectItem(j, "hava")) && hv_sicak) {
		hv_kod = (int)num(h, "kod", -1);
		lv_label_set_text(hv_sehir, sehir);
		lv_label_set_text_fmt(hv_sicak, "%.0f°", num(h, "sicaklik", 0));
		char du[48];
		snprintf(du, sizeof du, "%s", str(h, "durum", ""));
		if (du[0] >= 'a' && du[0] <= 'z')
			du[0] = (char)(du[0] - 32);	/* ilk harf buyuk (ASCII) */
		lv_label_set_text(hv_durum, du);
		lv_label_set_text_fmt(hv_detay, LV_SYMBOL_UP " %.0f°    " LV_SYMBOL_DOWN " %.0f°    Yağış %%%d    Rüzgâr %.0f km/s",
				      num(h, "max", 0), num(h, "min", 0), (int)num(h, "yagis", 0), num(h, "ruzgar", 0));
		cJSON *y = cJSON_GetObjectItem(h, "yarin");
		if (y)
			lv_label_set_text_fmt(hv_yarin, "Yarın: %s · %.0f° / %.0f° · yağış %%%d", str(y, "durum", ""),
					      num(y, "max", 0), num(y, "min", 0), (int)num(y, "yagis", 0));
		struct tm tm;
		localtime_r(&pv_zaman, &tm);
		lv_label_set_text_fmt(hv_saat, LV_SYMBOL_REFRESH " %02d:%02d güncellendi", tm.tm_hour, tm.tm_min);
		lv_obj_invalidate(hv_ikon);
		snprintf(ss_hava, sizeof ss_hava, "%s %.0f°C  %s", sehir, num(h, "sicaklik", 0), str(h, "durum", ""));
	}
	if (hv_sicak)
		hf_veri(j);	/* 0.45 haftalik serit */
	cJSON *nm = cJSON_GetObjectItem(j, "namaz"), *hb = cJSON_GetObjectItem(j, "haber");	/* 0.35 */
	if (cJSON_IsObject(nm))
		for (int i = 0; i < 6; i++)
			snprintf(pn_vakit[i], sizeof pn_vakit[i], "%s", str(nm, pn_vakit_ad[i], ""));
	if (cJSON_IsArray(hb)) {
		pn_haber_n = 0;
		cJSON *it;
		cJSON_ArrayForEach(it, hb)
			if (cJSON_IsString(it) && pn_haber_n < 8)
				snprintf(pn_haber_l[pn_haber_n++], sizeof pn_haber_l[0], "%s", it->valuestring);
	}
	if ((d = cJSON_GetObjectItem(j, "doviz")) && l_fx) {
		char t2[160] = "";
		for (int i = 0; i < 3; i++) {
			cJSON *v = cJSON_GetObjectItem(d, pv_para[i][0]);
			if (cJSON_IsNumber(v)) {
				char sy[24];
				snprintf(sy, sizeof sy, "%.2f", v->valuedouble);
				char *nk = strchr(sy, '.');
				if (nk)
					*nk = ',';	/* 0.33: Turkce ondalik virgul */
				snprintf(t2 + strlen(t2), sizeof t2 - strlen(t2), "%s%s  %s TL", t2[0] ? "\n" : "", pv_para[i][1], sy);
			}
		}
		if (t2[0])
			lv_label_set_text(l_fx, t2);
	}
	cJSON_Delete(j);
}

/* ------------------------------------------------------- 0.35 hatirlatici / alarm */
#define HT_DOSYA "/data/asistan/hatirlatici.txt"
#define HT_N 30
typedef struct { long z; char t; char m[204]; } ht_t;
static ht_t ht[HT_N];
static int ht_n, ht_yuklendi;

static void ht_kaydet(void)
{
	mkdir("/data/asistan", 0700);
	FILE *f = fopen(HT_DOSYA ".part", "w");
	if (!f)
		return;
	for (int i = 0; i < ht_n; i++)
		fprintf(f, "%ld|%c|%s\n", ht[i].z, ht[i].t, ht[i].m);
	if (fclose(f) == 0)
		rename(HT_DOSYA ".part", HT_DOSYA);
	sync();
}

static void ht_ekle(long z, char t, const char *m)
{
	if (ht_n >= HT_N || z <= 0)
		return;
	ht[ht_n].z = z;
	ht[ht_n].t = t == 'a' || t == 'g' ? t : 'h';
	snprintf(ht[ht_n].m, sizeof ht[0].m, "%s", m ? m : "");
	for (char *p = ht[ht_n].m; *p; p++)
		if (*p == '\n' || *p == '|')
			*p = ' ';
	ht_n++;
}

static void ht_yukle(void)
{
	FILE *f = fopen(HT_DOSYA, "r");
	char b[256];
	ht_n = 0;
	while (f && fgets(b, sizeof b, f)) {
		long z;
		char t;
		int n = 0;
		if (sscanf(b, "%ld|%c|%n", &z, &t, &n) == 2 && n > 0) {
			b[strcspn(b, "\n")] = 0;
			ht_ekle(z, t, b + n);
		}
	}
	if (f)
		fclose(f);
}

/* ------------------------------------------------------- 0.45 haftalik hava/takvim seridi (U2) */
#define HF_N 7
static lv_obj_t *hf_kolon[HF_N], *hf_gun[HF_N], *hf_ik[HF_N], *hf_der[HF_N], *hf_nok[HF_N];
static int hf_n, hf_kod[HF_N], hf_max[HF_N], hf_min[HF_N], hf_yag[HF_N];
static time_t hf_t0;	/* gun 0'in yerel ogle saati (yaz saatine karsi guvenli) */
static const char *const hf_kisa[7] = {"Paz", "Pzt", "Sal", "Çar", "Per", "Cum", "Cmt"};
static const char *const hf_uzun[7] = {"Pazar", "Pazartesi", "Salı", "Çarşamba", "Perşembe", "Cuma", "Cumartesi"};
static const char *const hf_ay[12] = {"Ocak", "Şubat", "Mart", "Nisan", "Mayıs", "Haziran", "Temmuz", "Ağustos", "Eylül",
				      "Ekim", "Kasım", "Aralık"};

static const char *hv_ad(int k)	/* WMO kodu -> kisa Turkce ad */
{
	if (k == 0)
		return "açık";
	if (k <= 2)
		return "az bulutlu";
	if (k == 3)
		return "bulutlu";
	if (k == 45 || k == 48)
		return "sisli";
	if (k >= 51 && k <= 57)
		return "çiseleme";
	if (k >= 61 && k <= 67)
		return "yağmurlu";
	if (k >= 71 && k <= 77)
		return "karlı";
	if (k >= 80 && k <= 82)
		return "sağanak";
	if (k == 85 || k == 86)
		return "kar sağanağı";
	if (k >= 95)
		return "fırtınalı";
	return "değişken";
}

/* gun i: yerel tarih ve gece yarisi; o gunku hatirlatici/alarm sayisi (ozet != NULL ise ilk ikisi) */
static int hf_hatirlatici(int i, char *ozet, size_t n)
{
	time_t g = hf_t0 + (time_t)i * 86400;
	struct tm t;
	localtime_r(&g, &t);
	t.tm_hour = t.tm_min = t.tm_sec = 0;
	t.tm_isdst = -1;
	time_t bas = mktime(&t), son = bas + 86400;
	int say = 0;
	if (ozet && n)
		ozet[0] = 0;
	for (int k = 0; k < ht_n; k++)
		if (ht[k].z >= (long)bas && ht[k].z < (long)son) {
			if (ozet && say < 2) {
				time_t z = (time_t)ht[k].z;
				struct tm hz;
				localtime_r(&z, &hz);
				size_t u = strlen(ozet);
				snprintf(ozet + u, n - u, "%s%02d:%02d %.24s", say ? ", " : "", hz.tm_hour, hz.tm_min,
					 ht[k].t == 'h' ? ht[k].m : "alarm");
			}
			say++;
		}
	return say;
}

static void hf_ciz(void)
{
	if (!hf_kolon[0])
		return;
	time_t now = time(NULL);
	struct tm bu;
	localtime_r(&now, &bu);
	for (int i = 0; i < HF_N; i++) {
		if (i >= hf_n) {
			lv_obj_add_flag(hf_kolon[i], LV_OBJ_FLAG_HIDDEN);
			continue;
		}
		lv_obj_remove_flag(hf_kolon[i], LV_OBJ_FLAG_HIDDEN);
		time_t g = hf_t0 + (time_t)i * 86400;
		struct tm t;
		localtime_r(&g, &t);
		int bugun = t.tm_year == bu.tm_year && t.tm_yday == bu.tm_yday;
		lv_label_set_text(hf_gun[i], bugun ? "Bugün" : hf_kisa[t.tm_wday % 7]);
		lv_obj_set_style_text_color(hf_gun[i], bugun ? C_ACC : C_DIM, 0);
		lv_label_set_text_fmt(hf_der[i], "%d° %d°", hf_max[i], hf_min[i]);
		if (hf_hatirlatici(i, NULL, 0))
			lv_obj_remove_flag(hf_nok[i], LV_OBJ_FLAG_HIDDEN);
		else
			lv_obj_add_flag(hf_nok[i], LV_OBJ_FLAG_HIDDEN);
		lv_obj_invalidate(hf_ik[i]);
	}
}

static void hf_veri(cJSON *j)
{
	cJSON *g = cJSON_GetObjectItem(j, "gunler"), *h = cJSON_GetObjectItem(j, "hava"), *it;
	hf_n = 0;
	if (cJSON_IsArray(g)) {
		cJSON_ArrayForEach(it, g) {
			if (hf_n >= HF_N)
				break;
			if (!cJSON_IsArray(it) || cJSON_GetArraySize(it) < 4)
				continue;
			int v[4];
			for (int k = 0; k < 4; k++) {
				cJSON *x = cJSON_GetArrayItem(it, k);
				v[k] = cJSON_IsNumber(x) ? (int)lround(x->valuedouble) : 0;
			}
			hf_kod[hf_n] = v[0];
			hf_max[hf_n] = v[1];
			hf_min[hf_n] = v[2];
			hf_yag[hf_n] = v[3];
			hf_n++;
		}
	} else if (cJSON_IsObject(h)) {	/* v11 verisi: bugun + yarin */
		hf_kod[0] = (int)num(h, "kod", -1);
		hf_max[0] = (int)lround(num(h, "max", 0));
		hf_min[0] = (int)lround(num(h, "min", 0));
		hf_yag[0] = (int)num(h, "yagis", 0);
		hf_n = 1;
		cJSON *y = cJSON_GetObjectItem(h, "yarin");
		if (cJSON_IsObject(y)) {
			hf_kod[1] = (int)num(y, "kod", -1);
			hf_max[1] = (int)lround(num(y, "max", 0));
			hf_min[1] = (int)lround(num(y, "min", 0));
			hf_yag[1] = (int)num(y, "yagis", 0);
			hf_n = 2;
		}
	}
	struct tm t;
	time_t z = pv_zaman ? pv_zaman : time(NULL);
	localtime_r(&z, &t);
	t.tm_hour = 12;
	t.tm_min = t.tm_sec = 0;
	t.tm_isdst = -1;
	hf_t0 = mktime(&t);
	hf_ciz();
}

static void hf_ik_ciz_cb(lv_event_t *e)
{
	lv_obj_t *o = (lv_obj_t *)lv_event_get_current_target(e);
	int i = (int)(uintptr_t)lv_obj_get_user_data(o);
	if (i < 0 || i >= hf_n)
		return;
	lv_area_t a;
	lv_obj_get_coords(o, &a);
	int32_t cy = (a.y1 + a.y2) / 2;
	hv_simge(lv_event_get_layer(e), (a.x1 + a.x2) / 2, cy, hf_kod[i], 0, 390, hv_zemin(cy));
}

static void toast_kayitsiz(const char *txt)	/* show_toast gibi, bildirim gecmisine yazmaz */
{
	int zaten = !lv_obj_has_flag(toast, LV_OBJ_FLAG_HIDDEN) && lv_obj_get_user_data(toast);
	lv_label_set_text(toast, txt);
	lv_obj_remove_flag(toast, LV_OBJ_FLAG_HIDDEN);
	lv_obj_move_foreground(toast);
	lv_obj_set_user_data(toast, (void *)(uintptr_t)(tick_ms() + 3500));
	if (!zaten)
		toast_anim(1);
	else {
		lv_anim_delete(toast, toast_opa_cb);
		lv_obj_set_style_opa(toast, LV_OPA_COVER, 0);
	}
}

static void hf_tik_cb(lv_event_t *e)
{
	int i = (int)(uintptr_t)lv_event_get_user_data(e);
	if (i < 0 || i >= hf_n)
		return;
	time_t g = hf_t0 + (time_t)i * 86400;
	struct tm t;
	localtime_r(&g, &t);
	char hz[128], s[300];
	int n = hf_hatirlatici(i, hz, sizeof hz);
	snprintf(s, sizeof s, LV_SYMBOL_BELL "  %s %d %s: %s, %d° / %d°, yağış %%%d", hf_uzun[t.tm_wday % 7], t.tm_mday,
		 hf_ay[t.tm_mon % 12], hv_ad(hf_kod[i]), hf_max[i], hf_min[i], hf_yag[i]);
	if (n) {
		size_t u = strlen(s);
		snprintf(s + u, sizeof s - u, " · %d hatırlatıcı: %s%s", n, hz, n > 2 ? "…" : "");
	}
	toast_kayitsiz(s);
}

static void hf_tick(lv_timer_t *t)	/* gun degisimi ve yeni hatirlaticilar */
{
	(void)t;
	hf_ciz();
}

static void hf_kur(lv_obj_t *kart)
{
	static int zamanlayici;
	hv_kart = kart;
	lv_obj_add_flag(hv_yarin, LV_OBJ_FLAG_HIDDEN);
	lv_obj_t *r = kutu(kart, LV_FLEX_FLOW_ROW, 4);
	lv_obj_set_style_pad_top(r, 4, 0);
	for (int i = 0; i < HF_N; i++) {
		lv_obj_t *k = hf_kolon[i] = lv_obj_create(r);
		lv_obj_remove_style_all(k);
		lv_obj_set_size(k, 0, LV_SIZE_CONTENT);
		lv_obj_set_flex_grow(k, 1);
		lv_obj_set_flex_flow(k, LV_FLEX_FLOW_COLUMN);
		lv_obj_set_flex_align(k, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
		lv_obj_set_style_pad_ver(k, 6, 0);
		lv_obj_set_style_pad_row(k, 0, 0);
		lv_obj_set_style_radius(k, 14, 0);
		lv_obj_set_style_bg_color(k, lv_color_hex(TH.sep), LV_STATE_PRESSED);
		lv_obj_set_style_bg_opa(k, LV_OPA_COVER, LV_STATE_PRESSED);
		lv_obj_remove_flag(k, LV_OBJ_FLAG_SCROLLABLE);
		lv_obj_add_flag(k, LV_OBJ_FLAG_CLICKABLE);
		lv_obj_add_event_cb(k, hf_tik_cb, LV_EVENT_CLICKED, (void *)(uintptr_t)i);
		hf_gun[i] = label(k, f_s, "");
		hf_ik[i] = lv_obj_create(k);
		lv_obj_remove_style_all(hf_ik[i]);
		lv_obj_set_size(hf_ik[i], 60, 52);
		lv_obj_remove_flag(hf_ik[i], LV_OBJ_FLAG_CLICKABLE);
		lv_obj_set_user_data(hf_ik[i], (void *)(uintptr_t)i);
		lv_obj_add_event_cb(hf_ik[i], hf_ik_ciz_cb, LV_EVENT_DRAW_MAIN_END, NULL);
		hf_der[i] = label(k, f_s, "");
		hf_nok[i] = lv_obj_create(k);
		lv_obj_remove_style_all(hf_nok[i]);
		lv_obj_set_size(hf_nok[i], 8, 8);
		lv_obj_set_style_radius(hf_nok[i], LV_RADIUS_CIRCLE, 0);
		lv_obj_set_style_bg_opa(hf_nok[i], LV_OPA_COVER, 0);
		lv_obj_set_style_bg_color(hf_nok[i], C_ACC, 0);
		lv_obj_set_style_margin_top(hf_nok[i], 3, 0);
		lv_obj_remove_flag(hf_nok[i], LV_OBJ_FLAG_CLICKABLE);
		lv_obj_add_flag(hf_nok[i], LV_OBJ_FLAG_HIDDEN);
		lv_obj_add_flag(k, LV_OBJ_FLAG_HIDDEN);
	}
	if (!zamanlayici) {
		zamanlayici = 1;
		lv_timer_create(hf_tick, 30000, NULL);
	}
	hf_ciz();
}

/* bildirim kuyrugu: autorun bunu Telegram'a yollar (jeton yoksa bekler) */
static void bk_yaz(const char *metin)
{
	static unsigned sira;
	mkdir("/tmp/bildirim-kuyruk", 0700);
	char y[96], g[112];
	snprintf(y, sizeof y, "/tmp/bildirim-kuyruk/%ld-%u.txt", (long)time(NULL), sira++);
	snprintf(g, sizeof g, "%s.part", y);
	FILE *f = fopen(g, "w");
	if (!f)
		return;
	fputs(metin, f);
	if (fclose(f) == 0)
		rename(g, y);
}

static void ht_cal(const ht_t *h)
{
	char s[300], tg[320];
	struct tm tm;
	time_t z = (time_t)h->z;
	localtime_r(&z, &tm);
	if (h->t == 'h') {
		snprintf(s, sizeof s, "Efendim, hatırlatıyorum: %s.", h->m);
		snprintf(tg, sizeof tg, "⏰ Hatırlatma (%02d:%02d): %s", tm.tm_hour, tm.tm_min, h->m);
	} else {
		snprintf(s, sizeof s, "Efendim, alarm! Saat %02d:%02d. Uyanma vakti efendim.", tm.tm_hour, tm.tm_min);
		snprintf(tg, sizeof tg, "⏰ Alarm: %02d:%02d — KÖLE", tm.tm_hour, tm.tm_min);
	}
	ky_konus(s, 0);
	ky_tut = 1;	/* hatirlatma/alarm yuzu acik kalir (dokununca kapanir) */
	ky_durum_metin = h->t == 'h' ? LV_SYMBOL_BELL "  Hatırlatma" : LV_SYMBOL_BELL "  Alarm";
	ky_ilerleme_yaz();
	show_toast(tg);
	bk_yaz(tg);
	printf("hatirlatici calindi: tur %c\n", h->t);
}

static void ht_tick(lv_timer_t *t)
{
	(void)t;
	time_t now = time(NULL);
	if (now < 1700000000)	/* saat henuz ayarli degil (NTP) */
		return;
	if (!ht_yuklendi) {
		ht_yukle();
		ht_yuklendi = 1;
	}
	int degisti = 0;
	for (int i = 0; i < ht_n; i++) {
		if (ht[i].z > now)
			continue;
		if (now - ht[i].z < 6 * 3600)	/* cihaz kapaliyken kacirilan: 6 saatten eskiyse sessiz dusur */
			ht_cal(&ht[i]);
		if (ht[i].t == 'g') {
			while (ht[i].z <= now)
				ht[i].z += 24 * 3600;
		} else {	/* 0.77: sira korunur ("sonuncuyu sil" eklenen son ogeyi siler) */
			memmove(&ht[i], &ht[i + 1], (size_t)(--ht_n - i) * sizeof ht[0]);
			i--;
		}
		degisti = 1;
		break;	/* ayni anda tek yuz: kalanlar sonraki tikte */
	}
	if (degisti)
		ht_kaydet();
}

static long ht_siradaki(char *m, size_t mn, char *tur)
{
	long en = 0;
	for (int i = 0; i < ht_n; i++)
		if (!en || ht[i].z < en) {
			en = ht[i].z;
			snprintf(m, mn, "%s", ht[i].m);
			*tur = ht[i].t;
		}
	return en;
}

/* 0.35 Pano: siradaki namaz vakti (dakikada bir) ve haber basligi (8 sn'de bir) */
static void pn_guncelle(void)
{
	static time_t son_haber;
	time_t now = time(NULL);
	if (pn_namaz && pn_vakit[0][0] && now > 1700000000) {
		struct tm tm;
		localtime_r(&now, &tm);
		int dk = tm.tm_hour * 60 + tm.tm_min, sec = -1, kalan = 0;
		for (int i = 0; i < 6; i++) {
			int h, m;
			if (sscanf(pn_vakit[i], "%d:%d", &h, &m) == 2 && h * 60 + m > dk) {
				sec = i;
				kalan = h * 60 + m - dk;
				break;
			}
		}
		char b[96];
		if (sec >= 0)
			snprintf(b, sizeof b, LV_SYMBOL_BELL "  %s %s · %d sa %d dk", pn_vakit_ad[sec], pn_vakit[sec], kalan / 60, kalan % 60);
		else
			snprintf(b, sizeof b, LV_SYMBOL_BELL "  Yarın imsak %s", pn_vakit[0]);
		gz_metin(pn_namaz, b);
	}
	if (pn_haber && pn_haber_n && now - son_haber >= 8) {
		son_haber = now;
		gz_metin(pn_haber, pn_haber_l[pn_haber_i++ % pn_haber_n]);
	}
}

/* DURUM karti: Hey Kole, kamera kaydi, PC (tick_cb'den saniyede bir) */
static void pd_guncelle(void)
{
	if (!pd_l)
		return;
	char hk[64] = "kapalı", d[700] = "";
	if (ky_asistan) {
		FILE *f = fopen(KY_DURUM, "r");
		if (f) {
			size_t n = fread(d, 1, sizeof d - 1, f);
			d[n] = 0;
			fclose(f);
		}
		snprintf(hk, sizeof hk, "%s", strstr(d, "\"mod\":\"dinliyor\"") ? "dinliyor" : strstr(d, "\"mod\":\"ogren\"") ? "öğreniyor"
					   : d[0] ? "beklemede" : "başlatılıyor");
	}
	char pc[96];
	if (g_pc_ok && l_cpu && l_ram) {	/* 0.68: kisa (DURUM kartinda tek satira sigsin), Turkce yuzde "%6" */
		char c1[16], c2[16];
		yuzde_tr(c1, sizeof c1, lv_label_get_text(l_cpu));
		yuzde_tr(c2, sizeof c2, lv_label_get_text(l_ram));
		snprintf(pc, sizeof pc, "bağlı · CPU %s · RAM %s", c1, c2);
	} else
		snprintf(pc, sizeof pc, "bağlı değil");
	char t2[520], hm[204] = "", htr = 'h', hs[260] = "";
	long hz = ht_siradaki(hm, sizeof hm, &htr);	/* 0.35 */
	if (hz) {
		struct tm tm;
		time_t z = (time_t)hz;
		localtime_r(&z, &tm);
		snprintf(hs, sizeof hs, "\n" LV_SYMBOL_BELL "  %02d.%02d %02d:%02d %s", tm.tm_mday, tm.tm_mon + 1, tm.tm_hour, tm.tm_min,
			 htr == 'h' ? hm : htr == 'g' ? "her gün alarm" : "alarm");
	}
	static time_t kam_t;	/* 0.46: kamera-kayit "kamera USB acilamadi" -> mod acik olsa da kayit yok */
	static int kam_yok;
	time_t kam_simdi = time(NULL);
	if (kam_simdi - kam_t >= 10 || kam_simdi < kam_t) {
		char kj[1024];
		kam_t = kam_simdi;
		kam_yok = dosya_oku(GZ_DURUM, kj, sizeof kj) > 0 && strstr(kj, "\"hata\":\"kamera USB") != NULL;
	}
	int kam_mod = g_kayit || dosya_var("/mnt/sd/kamera/bekci-acik") || dosya_var("/mnt/sd/kamera/zaman-acik");
	snprintf(t2, sizeof t2, LV_SYMBOL_AUDIO "  Hey Köle: %s\n" LV_SYMBOL_VIDEO "  Kamera kaydı: %s\n" LV_SYMBOL_DRIVE "  PC: %s%s", hk,
		 kam_mod && kam_yok ? "kamera yok" :
		 g_kayit ? (dosya_var("/mnt/sd/kamera/bekci-acik") ? "açık · bekçi" : "açık")
			 : dosya_var("/mnt/sd/kamera/bekci-acik") ? (dosya_var("/mnt/sd/kamera/zaman-acik") ? "bekçi + zaman atlamalı" : "bekçi açık")
			 : dosya_var("/mnt/sd/kamera/zaman-acik") ? "zaman atlamalı" : "kapalı", pc, hs);
	gz_metin(pd_l, t2);
}

/* ------------------------------------------------------- 0.32 sesli panel islemleri */
static long ey_son_seq = -1;
static time_t ey_mtime, ey_acilis;	/* 0.73: panelin acildigi an (ilk eylem dosyasi yutulmasin) */
static char ey_bekleyen[1024];	/* konusma bitene kadar bekleyen islem listesi (JSON) */

static int ey_izinli(const char *k)
{
	static const char *const izin[] = {"br_mode", "br_pct", "theme", "ss", "kilit", "dnd", "sayfa", "ayar", "not_ekle", "press", "cerceve",
					   "ses", "sessiz"};	/* 0.73: panel sesi (0.74 zaman_olay/zaman_aralik asagida ayri) */
	for (size_t i = 0; i < sizeof izin / sizeof *izin; i++)
		if (!strcmp(k, izin[i]))
			return 1;
	return 0;
}

static void ey_uygula(const char *liste)
{
	cJSON *ui = cJSON_Parse(liste), *it;
	if (!cJSON_IsArray(ui)) {
		cJSON_Delete(ui);
		return;
	}
	cJSON_ArrayForEach(it, ui) {
		if (!cJSON_IsObject(it))
			continue;
		cJSON *o;
		if ((o = cJSON_GetObjectItem(it, "tm"))) {	/* zamanlayici */
			if (cJSON_IsNumber(o) && o->valueint >= 1 && o->valueint <= 180) {
				tm_set("Sesli", o->valueint);
				tm_run = 1;
				tm_last_ms = tick_ms();
				tm_draw();
				show_toast(LV_SYMBOL_BELL "  Zamanlayıcı başladı");
			} else if (cJSON_IsString(o)) {
				if (!strcmp(o->valuestring, "dur"))
					tm_run = 0;
				else if (!strcmp(o->valuestring, "devam") && tm_left > 0) {
					tm_run = 1;
					tm_last_ms = tick_ms();
				} else if (!strcmp(o->valuestring, "sifirla")) {
					tm_left = tm_total;
					tm_run = 0;
				}
				tm_draw();
			}
			continue;
		}
		if (cJSON_IsString(o = cJSON_GetObjectItem(it, "kayit"))) {
			if (!strcmp(o->valuestring, "baslat"))
				gz_kayit_baslat();
			else if (!strcmp(o->valuestring, "durdur") && dosya_var(GZ_KAYIT_BAYRAK)) {
				unlink(GZ_KAYIT_BAYRAK);
				gz_gunluk("SD kaydı sesli komutla durduruldu");
				show_toast(LV_SYMBOL_STOP "  SD kaydı durduruldu");
			}
			continue;
		}
		if (cJSON_IsString(o = cJSON_GetObjectItem(it, "kamera_foto")) || cJSON_IsString(o = cJSON_GetObjectItem(it, "bekci")) ||
		    cJSON_IsString(o = cJSON_GetObjectItem(it, "zaman"))) {	/* 0.36 kamera (kamera-kayit v3) */
			const char *k = o->string, *v = o->valuestring;
			if (g_kalkan) {
				show_toast(LV_SYMBOL_EYE_CLOSE "  Gizlilik kalkanı açık: kamera kullanılamaz");
				continue;
			}
			if (!strcmp(k, "kamera_foto")) {
				FILE *ff = fopen("/tmp/kamera-foto-iste", "w");
				if (ff) {
					fputs(!strcmp(v, "telegram") ? "telegram\n" : "ekran\n", ff);
					fclose(ff);
				}
				show_toast(LV_SYMBOL_IMAGE "  Fotoğraf çekiliyor");
			} else {
				const char *y = !strcmp(k, "bekci") ? "/mnt/sd/kamera/bekci-acik" : "/mnt/sd/kamera/zaman-acik";
				int ac = !strcmp(v, "ac") || !strcmp(v, "baslat");
				if (ac) {
					mkdir("/mnt/sd/kamera", 0755);
					int fd = open(y, O_WRONLY | O_CREAT | O_TRUNC | O_CLOEXEC, 0644);
					if (fd >= 0)
						close(fd);
				} else {
					unlink(y);
				}
				sync();
				show_toast(!strcmp(k, "bekci") ? (ac ? LV_SYMBOL_EYE_OPEN "  Bekçi modu açık" : LV_SYMBOL_EYE_CLOSE "  Bekçi modu kapalı")
							      : (ac ? LV_SYMBOL_VIDEO "  Zaman atlamalı çekim başladı" : LV_SYMBOL_STOP "  Zaman atlamalı çekim durdu"));
			}
			continue;
		}
		if (cJSON_IsBool(o = cJSON_GetObjectItem(it, "heykole"))) {	/* 0.75 (kole-gemini v35): sesle yalniz kapatma */
			if (!cJSON_IsTrue(o) && ky_asistan)
				ky_asistan_ayarla(0, "sesli islem");
			continue;
		}
		if (cJSON_IsString(o = cJSON_GetObjectItem(it, "zaman_olay"))) {	/* 0.74 (kole-gemini v34) */
			if (!strcmp(o->valuestring, "ac") || !strcmp(o->valuestring, "kapat"))
				zl_olay_ayarla(!strcmp(o->valuestring, "ac"));
			continue;
		}
		if (cJSON_IsNumber(o = cJSON_GetObjectItem(it, "zaman_aralik"))) {	/* 0.74 */
			if (o->valueint >= 1 && o->valueint <= 3600) {
				zl_aralik_yaz(o->valueint);
				char tm_[64];
				snprintf(tm_, sizeof tm_, LV_SYMBOL_VIDEO "  Time-lapse aralığı %d sn", o->valueint);
				show_toast(tm_);
			}
			continue;
		}
		if (cJSON_IsObject(o = cJSON_GetObjectItem(it, "hatirlat"))) {	/* 0.35 */
			if (!ht_yuklendi) {
				ht_yukle();
				ht_yuklendi = 1;
			}
			ht_ekle((long)num(o, "z", 0), str(o, "t", "h")[0], str(o, "m", ""));
			ht_kaydet();
			show_toast(LV_SYMBOL_BELL "  Hatırlatıcı kuruldu");
			continue;
		}
		if (cJSON_IsString(o = cJSON_GetObjectItem(it, "hatirlat_sil"))) {
			if (!ht_yuklendi) {
				ht_yukle();
				ht_yuklendi = 1;
			}
			if (!strcmp(o->valuestring, "hepsi"))
				ht_n = 0;
			else if (ht_n > 0)
				ht_n--;
			ht_kaydet();
			show_toast(LV_SYMBOL_TRASH "  Hatırlatıcı silindi");
			continue;
		}
		if (cJSON_IsNumber(o = cJSON_GetObjectItem(it, "br_rel"))) {	/* 0.34: goreli parlaklik */
			int v = (br_auto ? 60 : br_manual) + o->valueint;
			v = v < 5 ? 5 : v > 100 ? 100 : v;
			char l2[80];
			snprintf(l2, sizeof l2, "{\"t\":\"ui\",\"br_mode\":\"manual\",\"br_pct\":%d}", v);
			apply_ui(l2);
			continue;
		}
		if (cJSON_IsNumber(o = cJSON_GetObjectItem(it, "ses_rel"))) {	/* 0.73: goreli panel sesi */
			int v = ses_duzey + o->valueint;
			v = v < 0 ? 0 : v > 100 ? 100 : v;
			char l2[48];
			snprintf(l2, sizeof l2, "{\"t\":\"ui\",\"ses\":%d}", v);
			apply_ui(l2);
			continue;
		}
		if (cJSON_IsString(o = cJSON_GetObjectItem(it, "kalkan"))) {
			if (!strcmp(o->valuestring, "ac"))	/* kapatma yalniz dokunarak */
				kalkan_ayarla(1);
			continue;
		}
		/* standart: yalniz izinli anahtarlar apply_ui'ye; kilit yalniz kilitler; press yalniz medya makrosu */
		cJSON *temiz = cJSON_CreateObject();
		cJSON_AddStringToObject(temiz, "t", "ui");
		cJSON *c;
		cJSON_ArrayForEach(c, it) {
			if (!ey_izinli(c->string))
				continue;
			if (!strcmp(c->string, "kilit") && !cJSON_IsTrue(c))
				continue;
			if (!strcmp(c->string, "press") && (!cJSON_IsString(c) ||
				(strcmp(c->valuestring, "macro:playpause") && strcmp(c->valuestring, "macro:next") &&
				 strcmp(c->valuestring, "macro:prev") && strcmp(c->valuestring, "macro:volup") &&
				 strcmp(c->valuestring, "macro:voldown") && strcmp(c->valuestring, "macro:mute"))))
				continue;
			cJSON_AddItemToObject(temiz, c->string, cJSON_Duplicate(c, 1));
		}
		char *line = cJSON_PrintUnformatted(temiz);
		cJSON_Delete(temiz);
		if (line) {
			printf("sesli islem: %s\n", line);
			apply_ui(line);
			free(line);
		}
	}
	cJSON_Delete(ui);
}

static void ey_tick(lv_timer_t *t)
{
	(void)t;
	if (ey_bekleyen[0] && !(ky_obj && ky_i < ky_n)) {	/* Kole konusmasini bitirdi: uygula */
		char l2[sizeof ey_bekleyen];
		snprintf(l2, sizeof l2, "%s", ey_bekleyen);
		ey_bekleyen[0] = 0;
		ey_uygula(l2);
	}
	struct stat st;
	if (!ey_acilis)
		ey_acilis = time(NULL);
	if (stat("/tmp/asistan-eylem.json", &st) || st.st_mtime == ey_mtime)
		return;
	ey_mtime = st.st_mtime;
	char b[1200];
	FILE *f = fopen("/tmp/asistan-eylem.json", "r");
	if (!f)
		return;
	size_t n = fread(b, 1, sizeof b - 1, f);
	b[n] = 0;
	fclose(f);
	cJSON *j = cJSON_Parse(b);
	cJSON *sq = j ? cJSON_GetObjectItem(j, "seq") : NULL, *ui = j ? cJSON_GetObjectItem(j, "ui") : NULL;
	if (cJSON_IsNumber(sq) && cJSON_IsArray(ui)) {
		long seq = (long)(sq->valuedouble / 1e6);
		if (ey_son_seq < 0 && st.st_mtime < ey_acilis)
			ey_son_seq = seq;	/* acilista eski islemi uygulama (0.73: yalniz panel acilmadan once yazildiysa) */
		else if (seq != ey_son_seq && ky_asistan) {
			ey_son_seq = seq;
			char *line = cJSON_PrintUnformatted(ui);
			if (line && strlen(line) < sizeof ey_bekleyen)
				snprintf(ey_bekleyen, sizeof ey_bekleyen, "%s", line);
			free(line);
		}
	}
	cJSON_Delete(j);
}

static void apply_state(const char *line)
{
	cJSON *j = cJSON_Parse(line), *o;
	char b1[32], b2[32];

	if (!j)
		return;
	if (strcmp(str(j, "t", "state"), "state")) {
		cJSON_Delete(j);
		return;
	}
	int taze = line != last_state;	/* 0.77: yeniden kurulumda tekrar uygulanan eski mesajin saati/bildirimi kullanilmaz */
	if (taze) {
		free(last_state);
		last_state = strdup(line);
	}
	double t = num(j, "time", 0);
	static double pc_kayma;	/* 0.79: saat gecerliyken PC'nin yaptigi toplam kayma (denetim v3 A11) */
	if (taze) {
		struct timeval now;
		gettimeofday(&now, NULL);
		if (saat_kayma_uygula((double)now.tv_sec, t, &pc_kayma)) {	/* 0.79: adim adim yillarca ileri alinamaz */
			struct timeval tv = {.tv_sec = (time_t)t};
			settimeofday(&tv, NULL);
		}
	}
	if ((o = cJSON_GetObjectItem(j, "pc"))) {
		lv_label_set_text(l_pcname, str(o, "name", "PC"));
		set_metric(l_cpu, b_cpu, num(o, "cpu", -1));
		set_metric(l_ram, b_ram, num(o, "ram", -1));
		set_metric(l_disk, b_disk, num(o, "disk", -1));
		fmt_rate(b1, sizeof b1, num(o, "down", 0));
		fmt_rate(b2, sizeof b2, num(o, "up", 0));
		lv_label_set_text_fmt(l_netio, LV_SYMBOL_DOWNLOAD " %s    " LV_SYMBOL_UPLOAD " %s", b1, b2);
		lv_label_set_text(l_pcup, str(o, "info", ""));
	}
	if ((o = cJSON_GetObjectItem(j, "weather")) && !pv_taze()) {	/* 0.32: cihaz verisi tazeyse PC'ninki kullanilmaz */
		lv_label_set_text_fmt(l_weather, "%s  %.0f°C", str(o, "city", ""), num(o, "temp", 0));
		lv_label_set_text(l_weather2, str(o, "desc", ""));
		snprintf(ss_hava, sizeof ss_hava, "%s %.0f°C  %s", str(o, "city", ""), num(o, "temp", 0), str(o, "desc", ""));
	}
	if ((o = cJSON_GetObjectItem(j, "fx")) && cJSON_IsObject(o) && !pv_taze()) {
		char t2[256] = "";
		cJSON *it;
		cJSON_ArrayForEach(it, o) {
			if (cJSON_IsNumber(it) && strlen(t2) < 200)
				snprintf(t2 + strlen(t2), sizeof t2 - strlen(t2), "%s%s  %.2f TL", t2[0] ? "\n" : "", it->string,
					 it->valuedouble);
		}
		lv_label_set_text(l_fx, t2[0] ? t2 : "-");
	}
	if ((o = cJSON_GetObjectItem(j, "events")) && cJSON_IsArray(o)) {
		char t2[1024] = "";
		cJSON *it;
		cJSON_ArrayForEach(it, o) {
			if (strlen(t2) < 900)
				snprintf(t2 + strlen(t2), sizeof t2 - strlen(t2), "%s" LV_SYMBOL_BELL " %s  %s", t2[0] ? "\n" : "",
					 str(it, "time", ""), str(it, "title", ""));
		}
		lv_label_set_text(l_events, t2[0] ? t2 : "Bugün etkinlik yok");
		cJSON *ilk = cJSON_GetArrayItem(o, 0);	/* 0.22: koruyucuda siradaki etkinlik */
		if (ilk)
			snprintf(ss_etkinlik, sizeof ss_etkinlik, LV_SYMBOL_BELL " %s  %s", str(ilk, "time", ""), str(ilk, "title", ""));
		else
			ss_etkinlik[0] = 0;
	}
	if (cJSON_GetObjectItem(j, "pc"))
		pc_state_t = mono_s();	/* 0.58 */
	if (cJSON_IsString(o = cJSON_GetObjectItem(j, "pc_ses")) && strcmp(pc_ses, o->valuestring)) {	/* 0.58 */
		snprintf(pc_ses, sizeof pc_ses, "%.7s", o->valuestring);
		pc_ses_ciz();
	}
	if ((o = cJSON_GetObjectItem(j, "macros")) && cJSON_IsArray(o)) {
		char *s = cJSON_PrintUnformatted(o);
		if (s && strncmp(s, macro_sig, sizeof macro_sig - 1)) {
			snprintf(macro_sig, sizeof macro_sig, "%s", s);
			build_buttons(macro_cont, o, 0, 360, 190);
		}
		free(s);
	}
	if ((o = cJSON_GetObjectItem(j, "prompts")) && cJSON_IsArray(o)) {
		pc_ajan_t = mono_s();	/* 0.52 */
		char *s = cJSON_PrintUnformatted(o);
		if (s && strncmp(s, prompt_sig, sizeof prompt_sig - 1)) {
			snprintf(prompt_sig, sizeof prompt_sig, "%s", s);
			build_buttons(prompt_cont, o, 1, 380, 96);
		}
		free(s);
	}
	if ((o = cJSON_GetObjectItem(j, "assistant")) && !as_gemini_var()) {	/* 0.69: PC "yerel ozet" Gemini'yi ezmesin */
		const char *a = str(o, "text", NULL);
		if (a)
			lv_label_set_text(l_answer, a);
		lv_label_set_text(l_asst_state, cJSON_IsTrue(cJSON_GetObjectItem(o, "busy")) ? "Yanıt bekleniyor..."
											 : str(o, "status", ""));
	}
	if (taze && (o = cJSON_GetObjectItem(j, "toast")) && cJSON_IsString(o) && o->valuestring[0])
		pc_toast(o->valuestring);	/* 0.23: Rahatsiz etme acikken yalniz gecmise */
	cJSON_Delete(j);
}

/* --------------------------------------------------------------- periyodik */
static void wlan_info(char *ip, size_t ipn, int *q)
{
	static int sig = 0;
	static time_t last;
	struct ifreq r;
	int s = socket(AF_INET, SOCK_DGRAM, 0);

	snprintf(ip, ipn, "-");
	*q = -1;
	memset(&r, 0, sizeof r);
	snprintf(r.ifr_name, IFNAMSIZ, "wlan0");
	if (s >= 0 && !ioctl(s, SIOCGIFADDR, &r))
		snprintf(ip, ipn, "%s", inet_ntoa(((struct sockaddr_in *)&r.ifr_addr)->sin_addr));
	if (s >= 0)
		close(s);
	/* /proc/net/wireless yok (WEXT kapali): 10 sn'de bir sinyal ve SSID. 0.76: once wpa_supplicant kontrol soketi
	 * (surec acilmaz), olmazsa eskisi gibi iw ciktisi */
	if (time(NULL) - last >= 10 || last == 0) {
		last = time(NULL);
		char wc[512], ws[2048], wv[64];
		sig = 0;
		snprintf(wl_ssid, sizeof wl_ssid, "-");
		if (strcmp(ip, "-") && !wpa_sor("/var/run/wpa_supplicant/wlan0", "/tmp/panel-wpa", "SIGNAL_POLL", wc, sizeof wc, 300) &&
		    !wpa_alan(wc, "RSSI", wv, sizeof wv) && !wpa_sor("/var/run/wpa_supplicant/wlan0", "/tmp/panel-wpa", "STATUS", ws,
		    sizeof ws, 300)) {
			sig = atoi(wv);
			if (wpa_alan(ws, "ssid", wl_ssid, sizeof wl_ssid))
				snprintf(wl_ssid, sizeof wl_ssid, "-");
			*q = sig ? sig : -1;
			return;
		}
		FILE *f = strcmp(ip, "-") ? popen("iw dev wlan0 link 2>/dev/null", "r") : NULL;
		char line[256];
		while (f && fgets(line, sizeof line, f)) {
			char *p;
			if ((p = strstr(line, "signal:")))
				sig = atoi(p + 7);
			else if ((p = strstr(line, "SSID:"))) {
				p += 5;
				while (*p == ' ')
					p++;
				p[strcspn(p, "\r\n")] = 0;
				snprintf(wl_ssid, sizeof wl_ssid, "%s", p);
			}
		}
		if (f)
			pclose(f);
	}
	*q = sig ? sig : -1;
}

static struct bt_status bt_durum_oku(void)	/* 0.78: bt_tick ve hizli bekci ortak */
{
	char data[1024];
	struct bt_status s = {0, -1, 0, 0};
	FILE *f = fopen("/tmp/bt-durum.json", "r");
	if (f) {
		size_t n = fread(data, 1, sizeof data - 1, f);
		int valid = !ferror(f) && n < sizeof data - 1;
		fclose(f);
		data[n] = 0;
		if (valid) {
			struct timespec now;
			clock_gettime(CLOCK_MONOTONIC, &now);
			s = bt_status_parse(data, now.tv_sec + now.tv_nsec / 1e9);
		}
	}
	return s;
}

/* 0.78: BT kapaliyken baglanti 200 ms'de bir yoklanir (mcu-servis durumu saniyede bir yazar: kopma <= ~1.2 sn) */
static void bt_hizli_bekci(void)
{
	static uint32_t son;
	if (!bt_kapali_mi() || tick_ms() - son < 150)	/* 0.79: 200 ms zamanlayicida < 200 yoklamalari atliyordu (B10) */
		return;
	son = tick_ms();
	struct bt_status s = bt_durum_oku();
	bt_bekci(s.fresh && s.connection == 1);
}

static void bt_tick(void)
{
	struct bt_status s = bt_durum_oku();
	bt_bekci(s.fresh && s.connection == 1);	/* 0.55 */
	if (!l_bt)
		return;
	if (!s.fresh) {
		gz_metin(l_bt, "Bluetooth bilgisi güncel değil");	/* 0.23: yalniz degisince boya */
		gz_renk(l_bt, C_DIM);
		return;
	}
	const char *connection = s.connection == 1 ? "Bağlı" :
				 s.connection == 0 ? "Bağlı değil" : "Bağlantı bilgisi bekleniyor";
	char bt[160];
	if (s.music_event)
		snprintf(bt, sizeof bt, "Bluetooth: %s\nMüzik bildirimi alındı (%.0f sn önce)", connection, s.music_age);
	else
		snprintf(bt, sizeof bt, "Bluetooth: %s\nMüzik bildirimi bekleniyor", connection);
	gz_metin(l_bt, bt);
	gz_renk(l_bt, s.connection == 1 ? C_OK : C_DIM);
}

static void tick_cb(lv_timer_t *t)
{
	static time_t last;
	time_t now = time(NULL);
	struct tm tm;
	char ip[32];
	int q;

	(void)t;
	cop_bosalt();	/* 0.23: gizlenmis perde/kilit/not penceresi burada (olay geri cagrisi disinda) silinir */
	pthread_mutex_lock(&mu);
	char *p = pending;
	pending = NULL;
	pthread_mutex_unlock(&mu);
	if (p) {
		apply_state(p);
		free(p);
	}
	pthread_mutex_lock(&mu);
	int un = ui_n;
	char *uq[8];
	memcpy(uq, ui_pending, sizeof uq);
	ui_n = 0;
	pthread_mutex_unlock(&mu);
	for (int i = 0; i < un; i++) {
		apply_ui(uq[i]);
		free(uq[i]);
	}
	uintptr_t until = (uintptr_t)lv_obj_get_user_data(toast);
	if (until && tick_ms() > until) {
		lv_obj_set_user_data(toast, NULL);
		if (pc_full)	/* PC tam ekranda: beklemeden gizle, dogrudan cizim hemen geri gelsin */
			lv_obj_add_flag(toast, LV_OBJ_FLAG_HIDDEN);
		else
			toast_anim(0);
	}
	tm_tick();
	wf_tick();
	sekme_izle();	/* 0.23 */
	bt_hizli_bekci();	/* 0.78 */
	if (now == last)
		return;
	last = now;
	static unsigned log_say;	/* 0.79: /tmp/panel.log RAM'de sinirsiz buyumesin (denetim v3 A13) */
	struct stat lst;
	if (++log_say % 60 == 0 && !fstat(1, &lst) && S_ISREG(lst.st_mode) && lst.st_size > 1024 * 1024) {
		fflush(stdout);
		if (!ftruncate(1, 0))
			lseek(1, 0, SEEK_SET);
		printf("panel.log 1 MB'i asti, kirpildi\n");
	}
	gizlilik_oku();	/* 0.22: once bayraklar (koruyucu ve serit ayni saniyede ayni durumu gostersin) */
	bt_tick();
	mon_ornekle();	/* 0.23: grafik gecmisi icin her saniye (ucuz: birkac /proc okumasi) */
	ss_tick();
	br_apply();
	localtime_r(&now, &tm);
	if (tm.tm_year > 120) {
		lv_label_set_text_fmt(l_clock, "%02d:%02d", tm.tm_hour, tm.tm_min);
		lv_label_set_text_fmt(l_sec, ":%02d", tm.tm_sec);
		lv_label_set_text_fmt(l_date, "%d %s %d, %s", tm.tm_mday, aylar[tm.tm_mon], tm.tm_year + 1900,
				      gunler[tm.tm_wday]);
		lv_label_set_text_fmt(l_hdr_date, "%d %s %s  %02d:%02d", tm.tm_mday, aylar[tm.tm_mon], gunler[tm.tm_wday],
				      tm.tm_hour, tm.tm_min);
		int sa = tm.tm_hour;
		gz_metin(l_selam, sa >= 5 && sa < 11 ? "GÜNAYDIN" : sa >= 11 && sa < 17 ? "İYİ GÜNLER" :
				  sa >= 17 && sa < 22 ? "İYİ AKŞAMLAR" : "İYİ GECELER");
	} else {
		lv_label_set_text(l_date, "Saat ayarlanmadı (Wi-Fi/NTP ya da PC ajanı bekleniyor)");
	}
	int pc_ok = pc_conn && mono_s() - pc_last < 10;
	g_pc_ok = pc_ok;
	pd_guncelle();	/* 0.32 DURUM karti */
	pn_guncelle();	/* 0.35 namaz + haber */
	hap_ayarla(&h_pc, pc_ok ? C_OK : C_DIM, 0, pc_ok ? "PC bağlı" : "PC yok");
	if (dnd || gece_aktif) {	/* 0.23; 0.41 gece kipi */
		hap_ayarla(&h_dnd, lv_color_hex(0x8B5CF6), 0, dnd ? "Rahatsız etme" : "Gece kipi");
		gizle(h_dnd.kap, 0);
	} else
		gizle(h_dnd.kap, 1);
	if (g_kam || g_mik) {
		char gt[64];
		snprintf(gt, sizeof gt, "%s%s%s", g_kam ? LV_SYMBOL_EYE_OPEN " KAMERA AÇIK" : "", g_kam && g_mik ? "  " : "",
			 g_mik ? LV_SYMBOL_AUDIO " MİKROFON" : "");
		hap_ayarla(&h_priv, lv_color_hex(g_kam ? 0xDC2626 : 0xEA580C), 1, gt);
		if (lv_obj_has_flag(h_priv.kap, LV_OBJ_FLAG_HIDDEN))
			lv_obj_remove_flag(h_priv.kap, LV_OBJ_FLAG_HIDDEN);
	} else if (!lv_obj_has_flag(h_priv.kap, LV_OBJ_FLAG_HIDDEN))
		lv_obj_add_flag(h_priv.kap, LV_OBJ_FLAG_HIDDEN);
	if (g_kalkan) {
		hap_ayarla(&h_kalkan, C_OK, 0, "Kalkan");
		if (lv_obj_has_flag(h_kalkan.kap, LV_OBJ_FLAG_HIDDEN))
			lv_obj_remove_flag(h_kalkan.kap, LV_OBJ_FLAG_HIDDEN);
	} else if (!lv_obj_has_flag(h_kalkan.kap, LV_OBJ_FLAG_HIDDEN))
		lv_obj_add_flag(h_kalkan.kap, LV_OBJ_FLAG_HIDDEN);
	if (tabview && lv_tabview_get_tab_active(tabview) == GZ_SEKME)
		gz_tick();
	if (now % 3 == 0 || l_sys) {
		wlan_info(ip, sizeof ip, &q);
		snprintf(g_ip, sizeof g_ip, "%s", ip);	/* 0.23: Denetim Merkezi / Monitor / Ayarlar */
		g_dbm = q;
		int bagli = strcmp(ip, "-") != 0, dz = bagli ? wifi_duzey(q) : 0;
		if (strcmp(lv_label_get_text(l_hdr_net), ip)) {
			lv_label_set_text(l_hdr_net, ip);
			lv_obj_set_style_text_color(l_hdr_net, bagli ? lv_color_hex(TH.text) : C_DIM, 0);
		}
		if (dz != wf_bar_duzey) {	/* yalniz duzey degisince boya */
			wf_bar_duzey = dz;
			for (int i = 0; i < 4; i++)
				lv_obj_set_style_bg_color(wf_bar[i], i < dz ? (dz <= 1 ? C_WARN : C_ACC) : lv_color_hex(TH.sep), 0);
		}
		struct sysinfo si;
		sysinfo(&si);
		char sig[24] = "";
		if (wifi_duzey(q))	/* 0.22: bilinmeyen sinyalde "(-1 dBm)" yazma */
			snprintf(sig, sizeof sig, " · %d dBm", q);
		lv_label_set_text_fmt(l_sys,
				      APP_ADI "\nSürüm %.*s\nWi-Fi: %s%s\nIP: %s\nPC: %s (%s)\nÇalışma: %ld sa %ld dk\n"
				      "Bellek: %lu/%lu MB boş\nYük: %.2f",	/* 0.53: dar sutunda tek satir */
				      (int)strcspn(PANEL_VER, " "), PANEL_VER, wl_ssid, sig, ip, pc_ok ? "bağlı" : "yok", pc_peer, si.uptime / 3600,
				      (si.uptime / 60) % 60, (unsigned long)(si.freeram * si.mem_unit >> 20),
				      (unsigned long)(si.totalram * si.mem_unit >> 20), si.loads[0] / 65536.0);
	}
	gece_tick();	/* 0.41 */
	if (sk_test_yokla())	/* 0.39 */
		printf("ses testi: sonuc %d (%s)\n", sk_son_test, sk_test_metni());
	if (tabview) {	/* 0.23: yalniz gorunen sayfa saniyede bir guncellenir */
		uint32_t ak = lv_tabview_get_tab_active(tabview);
		if (ak == S_MONITOR)
			mon_ciz();
		else if (ak == S_PCSIS)	/* 0.67 */
			pcs_ciz();
		else if (ak == S_MEDYA)
			medya_tick();
		else if (ak == S_AYARLAR)
			ayarlar_tick();
	}
	dm_guncelle();
	kilit_guncelle();
}

static volatile int quit;
static void on_sig(int s)
{
	(void)s;
	quit = 1;
}

int main(void)
{
	pthread_t th;

	setvbuf(stdout, NULL, _IOLBF, 0);
	if (!getenv("TZ"))
		setenv("TZ", "<+03>-3", 1);
	tzset();
	signal(SIGTERM, on_sig);
	signal(SIGINT, on_sig);
	signal(SIGPIPE, SIG_IGN);
	{	/* 0.23 test kancalari (yalniz PC/WSL provasi) */
		const char *vd = getenv("PANEL_VERI_DIZIN"), *pk = getenv("PANEL_PROC_KOK");
		if (vd && vd[0])
			snprintf(veri_dizin, sizeof veri_dizin, "%s", vd);
		if (pk && pk[0])
			snprintf(proc_kok, sizeof proc_kok, "%s", pk);
		if ((vd && vd[0]) || (pk && pk[0]))
			printf("test: veri %s, proc koku '%s'\n", veri_dizin, proc_kok);
		const char *rt = getenv("PANEL_RG_TOHUM"), *rgc = getenv("PANEL_RG_GECE");	/* 0.24 */
		if (rt && rt[0]) {
			rg_tohum_env = (uint32_t)strtoul(rt, NULL, 0);
			rg_tohum_var = 1;
		}
		if (rgc && (rgc[0] == '0' || rgc[0] == '1'))
			rg_gece_zorla = rgc[0] - '0';
		if ((rt && rt[0]) || rg_gece_zorla >= 0)
			printf("test: robot goz tohum %s, gece %d\n", rg_tohum_var ? rt : "-", rg_gece_zorla);
	}
	{	/* 0.58: hoparlor 19.10.2026'da (A3=1) dogrulandi -> Merkez/kopru "henuz calismiyor" uyarilari kalkar */
		char hb[200];
		veri_yol(hb, sizeof hb, "hoparlor-dogrulandi");
		if (!dosya_var(hb)) {
			FILE *f = fopen(hb, "w");
			if (f) {
				fprintf(f, "19.10.2026 A3=1 ile hoparlor dogrulandi (panel 0.58 yazdi)\n");
				fclose(f);
				printf("hoparlor dogrulandi bayragi yazildi: %s\n", hb);
			}
		}
	}
	if (fb_init())
		return 1;
	lv_init();
	lv_tick_set_cb(tick_ms);
	lv_display_t *d = lv_display_create(W, H);
	lv_display_set_color_format(d, LV_COLOR_FORMAT_XRGB8888);
	size_t bsz = (size_t)W * 60 * 4;
	void *lb0, *lb1;	/* 0.38: G2D icin ION tamponlari (olmazsa malloc) */
	if (!pg_hazirla(bsz)) {
		lb0 = pg_buf[0];
		lb1 = pg_buf[1];
	} else {
		lb0 = malloc(bsz);
		lb1 = malloc(bsz);
	}
	lv_display_set_buffers(d, lb0, lb1, bsz, LV_DISPLAY_RENDER_MODE_PARTIAL);
	lv_display_set_flush_cb(d, flush_cb);
	fonts_init();
	br_load();
	ses_codec_uygula();	/* 0.39 */
	tema_gocu_040();	/* 0.40 */
	{	/* 0.41 gece kipi ayari */
		char gy[160];
		veri_yol(gy, sizeof gy, "panel-gece-kapali");
		gece_ayar = access(gy, F_OK) != 0;
	}
	disp = d;
	lv_display_set_theme(d, lv_theme_default_init(d, C_ACC, lv_color_hex(TH.acc2), TH.dark, f_s));
	touch_init();
	lv_indev_t *in = lv_indev_create();
	lv_indev_set_type(in, LV_INDEV_TYPE_POINTER);
	lv_indev_set_read_cb(in, touch_read);
	notlar_yukle();	/* 0.23 */
	ky_asistan = !dosya_var(KY_KAPALI);	/* 0.29 Hey Kole: anahtarlar ui_build'den once */
	ky_altyazi = !dosya_var(KY_ALTYAZI_KAPALI);
	ky_yuz_yukle();	/* 0.30 */
	gc_acik = !dosya_var("/data/panel-gecis-kapali");	/* 0.33 */
	ekran_ters = !dosya_var("/data/panel-ekran-duz");	/* 0.47 */
	{	/* 0.65: PC baglanma kipi (yoksa Elle) */
		char y[160];
		veri_yol(y, sizeof y, "pc-baglan-oto");
		pc_oto = dosya_var(y);
	}
	printf("ekran 180 derece: %s\n", ekran_ters ? "acik" : "kapali");
	mon_ornekle();
	ui_build();
	ayar_esitle();
	acilis_animasyonu();	/* 0.43 */
	lv_timer_create(tick_cb, 200, NULL);
	lv_timer_create(ky_olay_tick, 250, NULL);	/* 0.29 */
	lv_timer_create(ey_tick, 300, NULL);	/* 0.32 sesli islemler */
	lv_timer_create(pv_tick, 5000, NULL);	/* 0.32 PC'siz Pano verisi */
	lv_timer_create(ht_tick, 5000, NULL);	/* 0.35 hatirlatici/alarm */
	lv_display_add_event_cb(d, refr_start_cb, LV_EVENT_REFR_START, NULL);
	lv_display_add_event_cb(d, gc_refr_cb, LV_EVENT_REFR_READY, NULL);	/* 0.33 gecis olcumu */
	lv_display_add_event_cb(d, rg_refr_cb, LV_EVENT_REFR_START, NULL);	/* 0.24 robot goz olcumu */
	lv_display_add_event_cb(d, rg_refr_cb, LV_EVENT_RENDER_READY, NULL);
	lv_display_add_event_cb(d, yz_refr_cb, LV_EVENT_REFR_START, NULL);	/* 0.62 yildiz olcumu */
	lv_display_add_event_cb(d, yz_refr_cb, LV_EVENT_RENDER_READY, NULL);
	{
		const char *e = getenv("PANEL_DOGRUDAN");
		pc_dc_izin = !(e && e[0] == '0');
		printf("PC dogrudan cizim: %s\n", pc_dc_izin ? "acik" : "kapali (PANEL_DOGRUDAN=0)");
		const char *g = getenv("PANEL_G2D_ALICI");
		pc_dg_acik = !(g && g[0] == '0');
		pc_dg_gen = (uint32_t)getpid() << 16 ^ tick_ms();
		pc_dg_yaz(0);
		printf("Aliciya G2D devri: %s\n", pc_dg_acik ? "acik" : "kapali (PANEL_G2D_ALICI=0)");
	}
	lv_timer_create(pc_image_tick,10,NULL);
	pthread_create(&th, NULL, net_thread, NULL);
	printf("%s %s hazir, port %d\n", APP_ADI, PANEL_VER, PORT);
	while (!quit) {
		uint32_t w = lv_timer_handler();
		uint32_t cap = pc_full ? 5 : 30;	/* tam ekranda kirli alan bildirimine hizli tepki */
		usleep((w > cap ? cap : w) * 1000);
	}
	int tty = open("/dev/tty1", O_RDWR);
	if (tty >= 0)
		ioctl(tty, KDSETMODE, KD_TEXT);
	return 0;
}
