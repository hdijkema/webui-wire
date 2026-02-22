window._web_wire_popup_menu = function(menu, x = -1, y = -1, kind = 'popup') {
    if (menu.id == '#f') { menu.id = null; }
    let menu_id = (kind == 'popup') ? '@@popup-menu@@' : '@@menubar@@';
    let submenu_els = [];
    let triggerMenuItem;
    let clearPopupMenu = function() {
        if (kind == 'popup') {
            let el = document.getElementById(menu_id);
            if (el !== null) {
                el.innerHTML = '';
                el.style.display = 'none';
            }
            if (menu.id !== null) {
                // Delay this trigger, because one could have choosen a menu item and we want this
                // to be triggered before the clear command is send.
                // But if no menu item has been selected, the clear command should
                // eventually be send.
                setTimeout(function () {
                    console.log("Sending clear trigger for menu clearance : " + menu.id);
                    let obj = { evt: 'menu-item-choosen', item: menu.id };
                    window._web_wire_put_evt(obj);
                }, 250);
            }
        } else {
            // hide all submenus
            submenu_els.forEach(function (el) { el.style.display = 'none'; });
        }
    };
    triggerMenuItem = function(id) {
        console.log("Triggering menu item : " + id);
        let obj = { evt: 'menu-item-choosen', item: id };
        window._web_wire_put_evt(obj);
    };
    let showSubMenu = function(menu_el, item_el, el, parent_type) {
        if (parent_type == 'menu') {
            el.style.display = 'flex';
            let rect = item_el.getBoundingClientRect();
            let r = rect.left;
            let t = rect.height;
            el.style.left = r + 'px';
            el.style.top = t + 'px';
        } else {
            el.style.display = "flex";
            let rect = menu_el.getBoundingClientRect();
            let irect =item_el.getBoundingClientRect();
            let r = rect.width + 5;
            let t = irect.y - rect.y;
            el.style.left =  r + "px";
            el.style.top = t + "px";
        }
    };
    let hideSubMenu = function(el) { el.style.display = "none"; };
    let makePopupMenu = function(el, menu, visible, type) {
        let i;
        let N = menu.length;
        for(i = 0; i < N; i++) {
            let item = menu[i];
            let item_el = document.createElement("div");
            item_el.id = item.id;
            item_el.classList.add("menu-item");
            let item_el_icon = document.createElement('span');
            item_el_icon.classList.add("menu-icon");
            if (item.icon) {
                let icon_img = document.createElement('img');
                icon_img.setAttribute('src', item.icon);
                item_el_icon.appendChild(icon_img);
            }
            if (item.separator) {
                item_el.classList.add("separator");
            }
            let item_el_name = document.createElement('span');
            item_el_name.classList.add('menu-name');
            item_el_name.innerHTML = item.name;
            let item_el_submenu = document.createElement('span');
            item_el_submenu.classList.add('menu-submenu');
            if (item.submenu) {
                if (type == 'submenu' || kind == 'popup') {
                    item_el_submenu.innerHTML = '&gt;';
                }
                item_el.setAttribute('type', 'submenu');
                let submenu_el = document.createElement("div");
                submenu_els.push(submenu_el);
                submenu_el.classList.add("submenu");
                submenu_el.classList.add("menu");
                item_el.appendChild(submenu_el);
                submenu_el.style.display = 'none';
                makePopupMenu(submenu_el, item.submenu.menu, false, 'submenu');
                item_el.addEventListener('mouseenter', function () { showSubMenu(el, item_el, submenu_el, type); });
                item_el.addEventListener('mouseleave', function () { hideSubMenu(submenu_el); });
            } else {
                item_el.setAttribute('type', 'item');
                item_el.addEventListener('click', function() { triggerMenuItem(item.id); });
            }
            item_el.appendChild(item_el_icon);
            item_el.appendChild(item_el_name);
            item_el.appendChild(item_el_submenu);
            el.appendChild(item_el);
        }
    };
    let el = document.getElementById(menu_id);
    if (el === null) {
        el = document.createElement("div");
        el.id = menu_id;
        el.classList.add((kind == 'popup') ? "popup-menu" : "menubar");
        if (kind == 'popup') {
            el.classList.add("menu");
            document.body.appendChild(el);
        } else {
            document.body.prepend(el);
        }
    } else {
        el.innerHTML = '';
    }
    makePopupMenu(el, menu.menu, true, 'menu');
    el.style.left = x + "px";
    el.style.top = y + "px";
    el.style.display = "flex";
    let clearer_f = function() {
        clearPopupMenu();
        document.body.removeEventListener('click', clearer_f);
        document.body.removeEventListener('contextmenu', clearer_f);
    };
    document.body.addEventListener('click', clearer_f);
    document.body.addEventListener('contextmenu', clearer_f);
};
window._web_wire_menu = function(menubar) {
    window._web_wire_popup_menu(menubar, -1, -1, 'menubar');
};
