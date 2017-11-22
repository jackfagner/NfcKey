/*
    Copyright (C) 2017  github.com/jackfagner

    This file is part of NfcKey.

    NfcKey is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    NfcKey is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with NfcKey.  If not, see <http://www.gnu.org/licenses/>.
*/
var NfcKey = function () { };

NfcKey.prototype = (function () {
    var intConst = [
        0x6D835AFC, 0x7D15CD97, 0x0942B409, 0x32F9C923, 0xA811FB02, 0x64F121E8,
        0xD1CC8B4E, 0xE8873E6F, 0x61399BBB, 0xF1B91926, 0xAC661520, 0xA21A31C9,
        0xD424808D, 0xFE118E07, 0xD18E728D, 0xABAC9E17, 0x18066433, 0x00E18E79,
        0x65A77305, 0x5AE9E297, 0x11FC628C, 0x7BB3431F, 0x942A8308, 0xB2F8FD20,
        0x5728B869, 0x30726D5A
    ];

    var transform = function (rotUid) {
        var intPos = 0, tmp1, tmp2;
        var v1 = ((((rotUid[3] << 24) >>> 0) | ((rotUid[2] << 16) >>> 0) | ((rotUid[1] << 8) >>> 0) | rotUid[0]) >>> 0) + intConst[intPos++];
        var v2 = ((((rotUid[7] << 24) >>> 0) | ((rotUid[6] << 16) >>> 0) | ((rotUid[5] << 8) >>> 0) | rotUid[4]) >>> 0) + intConst[intPos++];

        for (var i = 0; i < 12; i += 2) {
            tmp1 = rotateLeft((v1 ^ v2) >>> 0, v2 & 0x1F) + intConst[intPos++];
            tmp2 = rotateLeft((v2 ^ tmp1) >>> 0, tmp1 & 0x1F) + intConst[intPos++];
            v1 = rotateLeft((tmp1 ^ tmp2) >>> 0, tmp2 & 0x1F) + intConst[intPos++];
            v2 = rotateLeft((tmp2 ^ v1) >>> 0, v1 & 0x1F) + intConst[intPos++];
        }
        
        var r = new Uint8Array(8);
        r[0] = v1 & 0xFF;
        r[1] = (v1 >>> 8) & 0xFF;
        r[2] = (v1 >>> 16) & 0xFF;
        r[3] = (v1 >> 24) & 0xFF;
        r[4] = v2 & 0xFF;
        r[5] = (v2 >>> 8) & 0xFF;
        r[6] = (v2 >>> 16) & 0xFF;
        r[7] = (v2 >>> 24) & 0xFF;
        return r;
    };

    var rotateLeft = function (x, n) {
        return (((x << n) >>> 0) | (x >>> (32 - n))) >>> 0;
    };

    var parseHexUid = function (hexUid) {
        var r = new Uint8Array(8);
        if (!isValidHexUid(hexUid))
            return r;
        for (var i = 0; i < hexUid.length / 2; i++)
            r[i] = parseInt(hexUid.substr(i * 2, 2), 16);
        return r;
    };

    var isValidHexUid = function (hexUid) {
        return typeof hexUid === "string" && hexUid.length === 14;
    };

    var swap16 = function (val) {
        return ((((val & 0xFF) << 8) >>> 0)
            | ((val >>> 8) & 0xFF)) >>> 0;
    };

    var swap32 = function (val) {
        return ((((val & 0xFF) << 24) >>> 0)
            | (((val & 0xFF00) << 8) >>> 0)
            | ((val >>> 8) & 0xFF00)
            | ((val >>> 24) & 0xFF)) >>> 0;
    };

    var tohex = function (b, l) {
        var h = b.toString(16).toUpperCase();
        if (h.length % 2 !== 0)
            return '0' + h;
        return h;
    };

    return {

        constructor: NfcKey,

        getKey: function (uid) {
            if (!isValidHexUid(uid))
                return;
            var i;
            var uid8 = parseHexUid(uid);
            var rotUid = new Uint8Array(8);
            var rotation = ((uid8[1] + uid8[3] + uid8[5]) & 7) >>> 0;
            for (i = 0; i < 7; i++)
                rotUid[((i + rotation) & 7) >>> 0] = uid8[i];

            var transfUid = transform(rotUid);

            var intKey = 0;
            var offset = (transfUid[0] + transfUid[2] + transfUid[4] + transfUid[6]) & 3;
            for (i = 0; i < 4; i++)
                intKey = transfUid[i + offset] + ((intKey << 8) >>> 0);

            return tohex(swap32(intKey));
        },

        getPack: function (uid) {
            if (!isValidHexUid(uid))
                return;
            var i;
            var uid8 = parseHexUid(uid);
            var rotUid = new Uint8Array(8);
            var rotation = ((uid8[2] + uid8[5]) & 7) >>> 0;
            for (i = 0; i < 7; i++)
                rotUid[((i + rotation) & 7) >>> 0] = uid8[i];

            var transfUid = transform(rotUid);

            var intPack = 0;
            for (i = 0; i < 8; i++)
                intPack += transfUid[i] * 13;

            var res = ((intPack ^ 0x5555) >>> 0) & 0xFFFF;
            return tohex(swap16(res));
        }

    };
})();
