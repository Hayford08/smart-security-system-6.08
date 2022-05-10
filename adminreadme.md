# Admin Page overview

The purpose of the admin page is to let the admin give and remove access of users from certain esps.

# Routing to the admin page

To get to the admin page we use an html button of type submit, that when pressed sends us to another file called admin.py.

# Displaying Board Access

To display which esps each player has access to we do the following.

First we get all usernames for the user table.

The we select all the boards from the doors_user_table (which will contain all boards, since admin has access to all of them).

Then for each username, we get all the doors that the username has access to from the doors_users_table and put them in a set called allowed_doors.

Then for each door in the system, we check if it belongs in the allowed_doors set, if it does we display it in the html with a filled checkbox, and if it doesn't we display it with an unfilled checkbox.

For each user, the checkboxes, corresponding to the each door belong to a single form, with a single submit button that when pressed, sends a post request with the information of this form.

# changing Board access

To change access, you check which doors you want to give access to and you un-check all doors you want to remove access from.

Once you have done that for a single user, then you have to submit the user's button that will send a post request it the admin.py page, to a update the access accordingly.

Once the admin.py page receives the post request, it first checks that the post request doesn't correspond to the post request for accessing admin.py from interfacer.py. Then, it gets the username of the user the changes were made for. Then, it removes access to all doors for this username by deleting in the table doors_users_table, all entries with username as username. Then it iterates over all keys in the post response, that are not a username, and gives access to username for this doors, by calling add_door_access() which inserts into the door_users_table the door and the username, as shown in the code below.

``` python
for elt in request["form"]:
    if elt != "username":
        action = request["form"][elt]          
        if(action == "on"):
            add_door_access(int(elt), username)

```
# Returning to interfacer.py

Finally, we have a button that when clicked sends us back to the interfacer.py page.