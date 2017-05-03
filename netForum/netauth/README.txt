
ABOUT MODULE
------------

This module is used to get login/register vis SOAP API console netForum to Drupal 7 sites.

There are two person who contributed to build up this API module to access Drupal site through netForum.

With the help of this module user can easily register into Drupal site without having registration step in it but that user should have Avectra  netForum membership with or without member roles. 

Avectra netForum basically include two user roles i.e Member and Non-member. Through this module system can easily dectect the member type of user and get the SSO token to login into the site.


INSTALLATION
------------

1) Download the module file (compressed file) from the Drupal.org.

2) Unzip it into /sites/all/modules/netauth_sso or /sites/all/modules/contrib/netauth_sso.

3) Login to Drupal admin (back-end) panel.

4) Go to -> Modules -> FusionSpan -> netForum SSO and enable it.

5) After enabling, go to configuration of module and set WSDL url, Username and Password normally and SAVE settings.

6) Now your module is ready to go. Please test it with netForum account and try to login directly on site.

7) You can check the users in "People" tab, user will be created/registered with netForum credentials.