
var gN = function (name) { return Array.prototype.slice.call(document.getElementsByTagName(name)); };
var gI = function (id) { return document.getElementById(id); };
var lastUid = '';
var nfcKey = null;

document.addEventListener('DOMContentLoaded', function () {
    nfcKey = new NfcKey();

    gI('uid').value = '';
    gI('page0').value = '';
    gI('page1').value = '';

    gN('input').forEach(function (el, i) {
        el.addEventListener('keyup', fieldChange);
        el.addEventListener('change', fieldChange);
    });

});

function fieldChange(e) {
    var fieldId = e.target.id;
    if (fieldId === 'uid') {
        var uid = gI('uid').value.replace(/ /gi, '');
        if (isValidUid(uid)) {
            gI('page0').value = (uid.substr(0, 6) + calcChecksum(uid.substr(0, 6))).toUpperCase();
            gI('page1').value = uid.substr(6).toUpperCase();
        }
        else {
            gI('page0').value = '';
            gI('page1').value = '';
        }
    } else {
        var page0 = gI('page0').value.replace(/ /gi, '');
        var page1 = gI('page1').value.replace(/ /gi, '');
        if (page0.length === 8 && page1.length === 8) {
            gI('uid').value = (page0.substr(0, 6) + page1).toUpperCase();
        }
        else {
            gI('uid').value = '';
        }
    }

    var newUid = gI('uid').value.replace(/ /gi, '').toUpperCase();
    if (newUid !== lastUid) {
        lastUid = newUid;
        calcKeyPack(newUid);
    }
}

function isValidUid(uid) {
	if (!uid)
		return false;
	return new RegExp(/[A-F0-9]{14}/i).test(uid);
}

function calcChecksum(uid1) {
    if (uid1.length !== 6)
        return 'XX';
    var b1 = parseInt(uid1.substr(0, 2), 16);
    var b2 = parseInt(uid1.substr(2, 2), 16);
    var b3 = parseInt(uid1.substr(4, 2), 16);
    var cs = (b1 ^ b2 ^ b3 ^ 0x88).toString(16).toUpperCase();
    if (cs.length < 2)
      cs = '0' + cs;
    return cs;
}

function calcKeyPack(uid) {
    if (!isValidUid(uid)) {
        showData('', '', '');
        return;
    }
    var key = nfcKey.getKey(uid);
    var pack = nfcKey.getPack(uid);
    showData(uid, key, pack);
}

function showData(uid, key, pack) {
    document.querySelector('p.uid span').innerText = addSpace(uid);
    document.querySelector('p.key span').innerText = addSpace(key);
    document.querySelector('p.pack span').innerText = addSpace(pack);
    var disp = uid.length === 14 ? 'block' : '';
    document.querySelector('div.r').style.display = disp;
    document.querySelector('div.d').style.display = disp;
}

function addSpace(hex) {
	if (typeof hex !== 'string' || hex.length === 0)
		return '';
    var out = hex.substr(0, 2);
    for (var i = 2; i < hex.length; i += 2) {
        out += ' ' + hex.substr(i, 2);
    }
    return out;
}