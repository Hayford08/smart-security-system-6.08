function checkForSession(delete_session) {
    if (delete_session) {
        localStorage.clear();
        return;
    }
    const user = localStorage.getItem('name');
    const pass = localStorage.getItem('password');
    if (user !== null) {
        document.getElementById('username').value = user;
        document.getElementById('password').value = pass;
        document.getElementById('submit').click();
    }
}

function storeCredentials(username, password) {
    localStorage.setItem('name', username);
    localStorage.setItem('password', password);
}
