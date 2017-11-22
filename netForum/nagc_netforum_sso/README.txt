
--------

* Introduction
* Installation
* Requirements
* Configuration
* Login Scenarios


INTRODUCTION
-----------------
EASA_ProtechMX_SSO allows you to integrate a SSO functionality with Protech's Web Portal through their
authentication web services. This modules uses Protech defined Unified Login Scenario 2 – User logins into Drupal and
authenticates against Protech.


INSTALLATION
-----------------
EASA_ProtechMX_SSO can be installed like any other Drupal module -- place it in the modules directory for your site and
enable it.


REQUIREMENTS
-----------------
* PHP SOAP Libraries
* PHP 5.2 or higher

* Protech WDSL authentication web service URL
* Protech authentication security password


CONFIGURATION
-----------------
See the module configuration settings page at admin/config/people/easa_protechmx_sso. You can enter your WSDL authentication
webservice url, authentication security password, match a security group with a Drupal role (for example
Administrator -> admin) and match a Membership Type with a Drupal role (for example Member => member).


LOGIN SCENARIOS
-----------------
* Case 1:
User 1 Login
User 1 (uid=1) enters login credentials. Credentials authenticate against Drupal login validation only. Does not call
web service. If credentials are valid, user is login as a superuser in Drupal and not logged into Protech. If
credentials are invalid, login fails.

On logout, the user is logout out of Drupal. No web services are called.

* Case 2
Member Login – has logged in before
Member enters login credentials and it calls the web service to authenticate user - AuthenticateUser(securityPassword,
username, password). If user authenticates successfully, it sets/updates the returned token, first name, last name, in the
easa_protechmx_sso database table (created during installation), assigns the roles returned from Protech to the user
(MEMBER_TYPE_WEBROLE), sets the status=1, and updates id, email, and password.

If the login credentials fail to authenticate, it sets the Drupal user status to 0.

On logout, the user is logged out of Drupal, and the authentication web service is called -
DeleteUserSession(securityPassword, token) is called and the user is logged out of Protech.

* Case 3
Member Login – first time login
Member enters login credentials and it calls the web service to authenticate user -
AuthenticateUser(securityPassword, username, password). If user authenticates successfully, it sets the returned token,
first name, last name, in the easa_protechmx_sso database table (created during installation), assigns the roles returned
from Protech to the user (MEMBER_TYPE_WEBROLE), sets the status=1, and creates a user in Drupal with the id, email, and
password.

On logout, the user is logout out of Drupal, and the authentication web service is called -
DeleteUserSession(securityPassword, token) is called and the user is logged out of Protech.

* Case 4
Non Member Login – No longer a member (inactive) or set as a non member
Non member enters login credentials, and it calls the web service to authenticate user -
AuthenticateUser(securityPassword, username, password). If the login credentials fail to authenticate through the web
service, it sets the Drupal user status to 0 and does not log in user to either system.

If login credentials authenticate, but the MEMBER_TYPE_WEBROLE roles are different or removed, it updates
the role of the Drupal user during authentication with new roles.

On logout, the user is logout out of Drupal, and the authentication web service is called -
DeleteUserSession(securityPassword, token) is called and the user is logged out of Protech.